cmake_minimum_required(VERSION 3.25)

project(ZEROengineCore VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_VERBOSE_MAKEFILE ON)

# Sourcing
set(ZEROengineCore_Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GraphicalModule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ZEROengine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ZEROengineMurmurHash3.cpp
)

# module declaration
add_library(ZEROengineCore STATIC ${ZEROengineCore_Sources})
add_library(ZEROengine::ZEROengineCore ALIAS ZEROengineCore)

target_include_directories(ZEROengineCore
PUBLIC
 $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
)

# requiring atleast C++17
target_compile_features(ZEROengineCore PRIVATE cxx_std_17)
target_compile_options(ZEROengineCore PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)