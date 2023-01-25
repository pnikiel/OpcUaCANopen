set_property(TARGET ${EXECUTABLE} PROPERTY ENABLE_EXPORTS TRUE)
add_dependencies (Device Enumerator)

project(CanOpenOpcUa HOMEPAGE_URL "https://gitlab.cern.ch/atlas-dcs-opcua-servers/CanOpenOpcUa")
cmake_minimum_required(VERSION 3.10)

set(CPACK_PACKAGE_NAME "CanOpenOpcUa" )
set(CPACK_PACKAGE_VENDOR "CERN/ATLAS-DCS/pnikiel" )
set(CPACK_GENERATOR "RPM")
set(CPACK_PACKAGING_INSTALL_PREFIX "/opt/CanOpenOpcUa")
set(CPACK_RPM_PACKAGE_NAME "${CPACK_PACKAGE_NAME}" )

# Get type of Linux to not confuse CC7/ C8 RPMs
execute_process(COMMAND lsb_release -is OUTPUT_VARIABLE lsb_release_id OUTPUT_STRIP_TRAILING_WHITESPACE)
execute_process(COMMAND lsb_release -rs OUTPUT_VARIABLE lsb_release_release OUTPUT_STRIP_TRAILING_WHITESPACE)
if (lsb_release_id MATCHES "CentOS")
  if (lsb_release_release MATCHES "^8.")
    message(STATUS "Recognized CentOS 8.x")
    set(CPACK_SYSTEM_NAME "centos8")
  endif (lsb_release_release MATCHES "^8.")
  if (lsb_release_release MATCHES "^7.")
    message(STATUS "Recognized CC7 7.x")
    set(CPACK_SYSTEM_NAME "cc7")
  endif (lsb_release_release MATCHES "^7.")
endif (lsb_release_id MATCHES "CentOS")

# "Automatically" parse git describe into RPM version.
execute_process(COMMAND git describe --tags OUTPUT_VARIABLE git_describe_output OUTPUT_STRIP_TRAILING_WHITESPACE)
message("NOTE: git describe returned ${git_describe_output}")
string(REPLACE "." ";" versions_dot_sep "${git_describe_output}")
list(GET versions_dot_sep 0 CPACK_PACKAGE_VERSION_MAJOR)
list(GET versions_dot_sep 1 CPACK_PACKAGE_VERSION_MINOR)
list(GET versions_dot_sep 2 CPACK_PACKAGE_VERSION_PATCH)

# And also to the version indicator
file(WRITE ${PROJECT_SOURCE_DIR}/Server/include/version.h "#define VERSION_STR \"${git_describe_output}\"" )

# Create documentation

add_custom_command(
  OUTPUT ${PROJECT_SOURCE_DIR}/Documentation/AddressSpaceDoc.html
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMAND ${PYTHON_COMMAND} ${PROJECT_SOURCE_DIR}/quasar.py generate as_doc
  DEPENDS ${DESIGN_FILE}
  )

add_custom_target(as_doc DEPENDS ${PROJECT_SOURCE_DIR}/Documentation/AddressSpaceDoc.html )

add_custom_command(
  OUTPUT ${PROJECT_SOURCE_DIR}/Documentation/ConfigDocumentation.html
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMAND ${PYTHON_COMMAND} ${PROJECT_SOURCE_DIR}/quasar.py generate config_doc
  DEPENDS ${DESIGN_FILE}
  )

add_custom_target(config_doc DEPENDS ${PROJECT_SOURCE_DIR}/Documentation/ConfigDocumentation.html )

#SET( CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/opt" )

# post-install stuff
set(CPACK_RPM_POST_INSTALL_SCRIPT_FILE ${PROJECT_SOURCE_DIR}/RPM/scriptlets/post_install.sh)



install(TARGETS CanOpenOpcUa RUNTIME DESTINATION "${CPACK_PACKAGING_INSTALL_PREFIX}/bin" )
install(
  FILES
    ${PROJECT_SOURCE_DIR}/bin/ServerConfig.xml
    ${PROJECT_SOURCE_DIR}/bin/config.xml
  DESTINATION "${CPACK_PACKAGING_INSTALL_PREFIX}/bin")
install(
    FILES
    ${PROJECT_SOURCE_DIR}/Documentation/AddressSpaceDoc.html
    ${PROJECT_SOURCE_DIR}/Documentation/ConfigDocumentation.html
  DESTINATION "${CPACK_PACKAGING_INSTALL_PREFIX}/doc")

file(GLOB example_configuration_files ${PROJECT_SOURCE_DIR}/Documentation/ExampleConfiguration/* )
install(
    FILES
    ${example_configuration_files}
    DESTINATION "${CPACK_PACKAGING_INSTALL_PREFIX}/example_config"
)

install(FILES ${PROJECT_BINARY_DIR}/Configuration/Configuration.xsd DESTINATION "${CPACK_PACKAGING_INSTALL_PREFIX}/Configuration" )

include(CPack)

add_custom_target(rpm_with_docs DEPENDS as_doc config_doc package)
