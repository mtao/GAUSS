cmake_minimum_required(VERSION 3.2)

project (Optimization)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GAUSS_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GAUSS_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GAUSS_BINARY_DIR}/lib )

file(GLOB HEADER_CODE ${Optimization_SOURCE_DIR}/include/*.h)
file(GLOB SRC_CODE ${Optimization_SOURCE_DIR}/cpp/*.cpp)

set(GAUSS_INCLUDE_DIR ${GAUSS_INCLUDE_DIR}  ${Optimization_SOURCE_DIR}/include CACHE INTERNAL "" FORCE)

add_library(Optimization ${HEADER_CODE} ${SRC_CODE})
target_link_libraries(Optimization PUBLIC gauss_internal_libraries)
