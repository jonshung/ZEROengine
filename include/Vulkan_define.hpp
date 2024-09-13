#ifndef ZEROENGINE_VULKAN_DEFINE_H
#define ZEROENGINE_VULKAN_DEFINE_H

#include <vulkan/vulkan.hpp>

struct VkQueueInfo {
    VkQueue queue;
    uint32_t queueFamilyIndex;
};

struct VulkanSecondaryCommandBuffer {
    VkCommandBuffer buffer;
};

#endif // #ifndef ZEROENGINE_VULKAN_DEFINE_H