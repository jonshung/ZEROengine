cmake_minimum_required(VERSION 3.25)

project(ZEROengineTests VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_VERBOSE_MAKEFILE ON)

# module declaration
add_executable(ZEROengineTests)

# requiring atleast C++17
target_compile_features(ZEROengineTests PRIVATE cxx_std_17)
target_compile_options(ZEROengineTests PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)
target_compile_definitions(ZEROengineTests PUBLIC BINARY_PATH="${CMAKE_CURRENT_BINARY_DIR}")

# Sourcing
target_sources(ZEROengineTests
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
)

target_link_libraries(ZEROengineTests
PRIVATE
    ZEROengine::ZEROengineCore
    ZEROengine::ZEROengineSDL
    ZEROengine::ZEROengineVulkan
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests_asset/frag.spv ${CMAKE_CURRENT_BINARY_DIR}/frag.spv COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests_asset/vert.spv ${CMAKE_CURRENT_BINARY_DIR}/vert.spv COPYONLY)