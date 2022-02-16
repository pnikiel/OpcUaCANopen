if (NOT ENUMERATOR_SCOPE)
    message(FATAL_ERROR "To use Enumerator, you need to set ENUMERATOR_SCOPE variable e.g. in ProjectSettings")
endif()

add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/Enumerator/src/Enumerator.cpp ${PROJECT_BINARY_DIR}/Enumerator/include/Enumerator.hpp
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMAND ${PYTHON_COMMAND} Enumerator/generate.py --output_dir ${PROJECT_BINARY_DIR}/Enumerator --scope ${ENUMERATOR_SCOPE}
    DEPENDS ${DESIGN_FILE} templates/designToEnumeratorHeader.jinja templates/designToEnumeratorBody.jinja
    )

add_library (Enumerator OBJECT 
    ${PROJECT_BINARY_DIR}/Enumerator/src/Enumerator.cpp
    ${PROJECT_BINARY_DIR}/Enumerator/include/Enumerator.hpp)