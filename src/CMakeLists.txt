# Include directories for header files
include_directories(${CMAKE_SOURCE_DIR}/libs/argtable3/)
include_directories(${CMAKE_SOURCE_DIR}/libs/embedded_cli/)
include_directories(${CMAKE_SOURCE_DIR}/libs/wsconsole/)

# Add the executable target
add_executable(arg3cli main.c)

# Define the directories containing the source files
set(ARGTABLE3_SOURCE_DIR ${CMAKE_SOURCE_DIR}/libs/argtable3)
set(EMBEDDED_CLI_SOURCE_DIR ${CMAKE_SOURCE_DIR}/libs/embedded_cli)
set(WSCONSOLE_SOURCE_DIR ${CMAKE_SOURCE_DIR}/libs/wsconsole)

# Use file(GLOB ...) to create a list of all C source files for argtable3 and embedded_cli
file(GLOB ARGTABLE3_SOURCES "${ARGTABLE3_SOURCE_DIR}/*.c")
file(GLOB EMBEDDED_CLI_SOURCES "${EMBEDDED_CLI_SOURCE_DIR}/*.c")
file(GLOB WSCONSOLE_SOURCES "${WSCONSOLE_SOURCE_DIR}/*.c")

# Link the argtable3 and embedded_cli libraries by passing the list of source files to target_sources(...)
target_sources(arg3cli PRIVATE ${ARGTABLE3_SOURCES} ${EMBEDDED_CLI_SOURCES} ${WSCONSOLE_SOURCES})

# Link any required libraries
target_link_libraries(arg3cli PRIVATE m)
