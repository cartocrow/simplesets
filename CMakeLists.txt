cmake_minimum_required(VERSION 3.16)

set(CMAKE_INSTALL_BINARY_DIR bin CACHE STRING "Installation subdirectory for compiled executables and shared libraries.")
set(CMAKE_INSTALL_LIBRARY_DIR lib CACHE STRING "Installation subdirectory for compiled static libraries.")
set(CMAKE_INSTALL_SOURCE_DIR include CACHE STRING "Installation subdirectory for source code.")
set(CMAKE_INSTALL_SCRIPT_DIR script CACHE STRING "Installation subdirectory for scripts.")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(INSTALL_CONFIG_DIR ${CMAKE_INSTALL_LIBRARY_DIR}/cmake)
set(INSTALL_BINARY_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINARY_DIR})
set(INSTALL_SCRIPT_DIR ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SCRIPT_DIR})

set(CARTOCROW_INSTALL_DIR "${CMAKE_INSTALL_SOURCE_DIR}/cartocrow")

project(cartocrow-simplesets
    VERSION 1.0.0
    DESCRIPTION "CartoCrow SimpleSets"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CartoCrow REQUIRED)
### DEPENDENCIES ###
if(NOT EMSCRIPTEN)
    # Let CMake automatically handle Qt's "Meta-Object Compiler" preprocessor
    set(CMAKE_AUTOMOC ON)
endif()

# All source files should use include paths relative to the source root
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Not necessary for cartocrow/core version > v0.1.0
include_directories(${CavalierContours_INCLUDE_DIR})

add_subdirectory(library)
if (EMSCRIPTEN)
    add_subdirectory(wasm_frontend)
else()
    add_subdirectory(frontend)
    add_subdirectory(test)
endif()