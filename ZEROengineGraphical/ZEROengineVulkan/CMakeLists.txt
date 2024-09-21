cmake_minimum_required(VERSION 3.25)

project(ZEROengineVulkan VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_VERBOSE_MAKEFILE ON)

# required libraries
find_package(Vulkan REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(X11 REQUIRED)

set(ZEROengineVulkan_Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VmaUsage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanBasicScreenRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanGraphicalModule.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanGraphicsPipelineBuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanGraphicsPipelineTemplate.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanRenderContext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanRenderer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/VulkanWindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/windowing/X11/VulkanXCBWindow.cpp
)

# module declaration
add_library(ZEROengineVulkan STATIC ${ZEROengineVulkan_Sources})
add_library(ZEROengine::ZEROengineVulkan ALIAS ZEROengineVulkan)

# requiring atleast C++17
target_compile_features(ZEROengineVulkan PRIVATE cxx_std_17)
target_compile_options(ZEROengineVulkan PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

# Supressing VulkanMemoryUsage warnings
set_source_files_properties(src/VmaUsage.cpp
    PROPERTIES COMPILE_FLAGS "-Wno-implicit-fallthrough -Wno-unused-variable -Wno-parentheses -Wno-unused-function -Wno-unused-parameter -Wno-missing-field-initializers")

target_include_directories(ZEROengineVulkan
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>/include
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include/ZEROengine/ZEROengineGraphical/ZEROengineVulkan>
)

target_link_libraries(ZEROengineVulkan
PUBLIC
    ZEROengine::ZEROengineCore
    Vulkan::Vulkan
    glm::glm
)

if(WIN32)
    target_compile_definitions(ZEROengineVulkan PRIVATE VK_USE_PLATFORM_WIN32_KHR)
elseif(ANDROID)
    target_compile_definitions(ZEROengineVulkan PRIVATE VK_USE_PLATFORM_ANDROID_KHR)
elseif(OGRE_USE_WAYLAND)
    target_compile_definitions(ZEROengineVulkan PRIVATE VK_USE_PLATFORM_WAYLAND_KHR)
else()
    target_compile_definitions(ZEROengineVulkan PRIVATE VK_USE_PLATFORM_XCB_KHR)
    target_link_libraries(ZEROengineVulkan PUBLIC xcb ${X11_LIBRARIES})
endif()