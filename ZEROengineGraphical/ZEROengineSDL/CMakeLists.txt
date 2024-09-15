cmake_minimum_required(VERSION 3.25)

project(ZEROengineSDL VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_VERBOSE_MAKEFILE ON)

# required libraries
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)

# Sourcing
set(ZEROengineSDL_Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SDL_VulkanWindowContext.cpp
)

# module declaration
add_library(ZEROengineSDL STATIC ${ZEROengineSDL_Sources})
add_library(ZEROengine::ZEROengineSDL ALIAS ZEROengineSDL)

target_include_directories(ZEROengineSDL
PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
)
target_link_libraries(ZEROengineSDL PUBLIC 
  ZEROengine::ZEROengineCore 
  ZEROengine::ZEROengineVulkan
  SDL3::SDL3
)

# requiring atleast C++17
target_compile_features(ZEROengineSDL PRIVATE cxx_std_17)
target_compile_options(ZEROengineSDL PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)