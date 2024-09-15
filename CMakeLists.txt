cmake_minimum_required(VERSION 3.25)

# making sure shared libraries should be in binary folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/lib/ZEROengineCore")

project(ZEROengine VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Core
add_subdirectory(ZEROengineCore)

# Graphical Module
add_subdirectory(ZEROengineGraphical)

# Tests
add_subdirectory(tests)