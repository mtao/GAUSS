
cmake_minimum_required(VERSION 3.2)

project (EigenFit)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GAUSS_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GAUSS_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GAUSS_BINARY_DIR}/lib )

file(GLOB HEADER_CODE ${EigenFit_SOURCE_DIR}/include/*.h)
file(GLOB SRC_CODE ${EigenFit_SOURCE_DIR}/cpp/*.cpp)

#set(GAUSS_INCLUDE_DIR ${GAUSS_INCLUDE_DIR}  ${EigenFit_SOURCE_DIR}/include CACHE INTERNAL "" FORCE)
target_include_directories(gauss_internal_libraries INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

#include_directories(${GAUSS_INCLUDE_DIR})
add_library(EigenFit ${HEADER_CODE} ${SRC_CODE})
target_link_libraries(EigenFit PUBLIC gauss_internal_libraries)
