
cmake_minimum_required(VERSION 3.2)

project (Core)
set(CMAKE_CXX_STANDARD 14)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GAUSS_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GAUSS_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GAUSS_BINARY_DIR}/lib )

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

file(GLOB HEADER_CODE ${Core_SOURCE_DIR}/include/*.h)
file(GLOB SRC_CODE ${Core_SOURCE_DIR}/cpp/*.cpp)

#include_directories(${GAUSS_INCLUDE_DIR})
add_library(Core ${HEADER_CODE} ${SRC_CODE})
target_link_libraries(Core PUBLIC gauss_internal_libraries)
