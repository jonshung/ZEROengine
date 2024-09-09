cmake_minimum_required(VERSION 3.25)

# making sure shared libraries should be in binary folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

project(Proj_1 VERSION 1.0.0 LANGUAGES CXX)
set(TARGET_MAIN p_x86)
set(CMAKE_VERBOSE_MAKEFILE ON)

# required libraries
find_package(PkgConfig REQUIRED)
find_package(Vulkan REQUIRED)
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3)
find_package(glm CONFIG REQUIRED)

add_executable(${TARGET_MAIN})
# requiring atleast C++17
target_compile_features(${TARGET_MAIN} PRIVATE cxx_std_17)
target_compile_options(${TARGET_MAIN} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

target_sources(${TARGET_MAIN}
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/BasicApp.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/SDLWindowContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanRenderer.cpp
)

target_include_directories(${TARGET_MAIN} 
PRIVATE
    ${PROJECT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    SDL3::Headers
    ${VULCAN_INCLUDE_DIRS}
)

target_link_libraries(${TARGET_MAIN}
PUBLIC
    Vulkan::Vulkan
    SDL3::SDL3
    glm::glm
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/project_env.h.in ${CMAKE_CURRENT_BINARY_DIR}/project_env.h @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/frag.spv ${CMAKE_CURRENT_BINARY_DIR}/frag.spv COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tests/vert.spv ${CMAKE_CURRENT_BINARY_DIR}/vert.spv COPYONLY)