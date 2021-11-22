## ----------------------------------------------------
## Please see Documentation/quasarBuildSystem.html for
## information how to use this file.
## ----------------------------------------------------

set(CUSTOM_SERVER_MODULES CanModule CANopen )
set(EXECUTABLE OpcUaServer)
set(SERVER_INCLUDE_DIRECTORIES CanModule/CanInterfaceImplementations/sockcan )
set(SERVER_LINK_LIBRARIES -lsocketcan )
set(SERVER_LINK_DIRECTORIES  )

##
## If ON, in addition to an executable, a shared object will be created.
##
set(BUILD_SERVER_SHARED_LIB OFF)

##
## Add here any additional boost libraries needed with their canonical name
## examples: date_time atomic etc.
## Note: boost paths are resolved either from $BOOST_ROOT if defined or system paths as fallback
##
set(ADDITIONAL_BOOST_LIBS filesystem )

add_definitions(-fPIC)

#set(CANMODULE_BUILD_VENDORS ON)
#set(CANMODULE_BUILD_ANAGATE OFF)

#include_directories(
#    CanModule/CanLibLoader/include
#    CanModule/CanInterface/include
#)
