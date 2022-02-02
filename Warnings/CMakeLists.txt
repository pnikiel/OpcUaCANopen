add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/Warnings/Warnings.cpp
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMAND ${PYTHON_COMMAND} Warnings/generate.py --output_dir ${PROJECT_BINARY_DIR}/Warnings
    DEPENDS ${DESIGN_FILE} templates/designToWarnings.jinja
    )

add_library (Warnings OBJECT ${PROJECT_BINARY_DIR}/Warnings/Warnings.cpp)