add_library(CANopen OBJECT 
    src/NodeGuarding.cpp
    src/FrameFactory.cpp
    src/Sdo.cpp
    src/NodeStateEngine.cpp
    src/EmergencyErrors.cpp
    )

add_dependencies (CANopen Enumerator)