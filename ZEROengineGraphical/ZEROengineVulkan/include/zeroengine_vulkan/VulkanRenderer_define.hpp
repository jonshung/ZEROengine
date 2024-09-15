#ifndef ZEROENGINE_VULKAN_RENDERER_DEF_H
#define ZEROENGINE_VULKAN_RENDERER_DEF_H

#include <vulkan/vulkan.hpp>
#include "glm/glm.hpp"

#include <cstdint>

struct VulkanRendererSettings {
    bool enable_depth_stencil_pass = false;
};

#endif // #ifndef VULKAN_RENDERER_DEF_H