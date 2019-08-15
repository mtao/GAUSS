
cmake_minimum_required(VERSION 3.2)

project (SolversLinear)
set(CMAKE_CXX_STANDARD 14)
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${GAUSS_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${GAUSS_BINARY_DIR}/lib )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${GAUSS_BINARY_DIR}/lib )

#Second, for multi-config builds (e.g. msvc)
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${GAUSS_BINARY_DIR}/bin )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${GAUSS_BINARY_DIR}/pants )
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )


file(GLOB HEADER_CODE ${SolversLinear_SOURCE_DIR}/include/*.h)
file(GLOB SRC_CODE ${SolversLinear_SOURCE_DIR}/cpp/*.cpp)

#set(GAUSS_INCLUDE_DIR ${GAUSS_INCLUDE_DIR}  ${SolversLinear_SOURCE_DIR}/include CACHE INTERNAL "" FORCE)
target_include_directories(gauss_internal_libraries INTERFACE ${SolversLinear_SOURCE_DIR}/include)

#include_directories(${GAUSS_INCLUDE_DIR})
add_library(SolversLinear ${HEADER_CODE} ${SRC_CODE})
target_link_libraries(SolversLinear PUBLIC gauss_internal_libraries)
