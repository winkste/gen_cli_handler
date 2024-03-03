# Include the argtable3 library
include_directories(${CMAKE_SOURCE_DIR}/libs/argtable3/)

# Add the executable target
add_executable(arg3cli main.c)

# Define the directory containing the source files
set(ARGTABLE3_SOURCE_DIR ${CMAKE_SOURCE_DIR}/libs/argtable3)

# Use file(GLOB ...) to create a list of all C source files
file(GLOB ARGTABLE3_SOURCES "${ARGTABLE3_SOURCE_DIR}/*.c")

# Link the argtable3 library by passing the list of source files to target_sources(...)
target_sources(arg3cli PRIVATE ${ARGTABLE3_SOURCES})

# Link any required libraries
target_link_libraries(arg3cli PRIVATE m)
