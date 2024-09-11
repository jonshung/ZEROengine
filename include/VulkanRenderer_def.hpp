#ifndef VULKAN_RENDERER_DEF_H
#define VULKAN_RENDERER_DEF_H

#include <vulkan/vulkan.hpp>
#include "glm/glm.hpp"

#include <cstdint>

#include "VulkanContext_def.hpp"

struct VulkanRendererSettings {
};

struct VulkanRenderContextCreateInfo {
    VkQueueInfo queue_info;
    VkDevice device;
    uint32_t submission_queue_count;
};

struct BaseVertex {
    glm::vec2 v_pos;
    glm::vec3 b_color;
    
    static VkVertexInputBindingDescription bindingDescription() {
        VkVertexInputBindingDescription info {};
        info.binding = 0;
        info.stride = sizeof(BaseVertex);
        info.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return info;
    }

    static std::array<VkVertexInputAttributeDescription, 2> attributeDescription() {
        std::array<VkVertexInputAttributeDescription, 2> info {};
        info[0].binding = 0;
        info[0].location = 0;
        info[0].format = VK_FORMAT_R32G32_SFLOAT;
        info[0].offset = offsetof(BaseVertex, v_pos); // 0
        info[1].binding = 0;
        info[1].location = 0;
        info[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        info[1].offset = offsetof(BaseVertex, b_color); // 32 + 32 + 32
        return info;
    }
};
#endif // #ifndef VULKAN_RENDERER_DEF_H