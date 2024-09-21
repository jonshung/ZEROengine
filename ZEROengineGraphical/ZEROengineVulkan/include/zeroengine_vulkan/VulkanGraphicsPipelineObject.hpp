#ifndef ZEROENGINE_VULKAN_GRAPHICS_PIPELINE_OBJECT_H
#define ZEROENGINE_VULKAN_GRAPHICS_PIPELINE_OBJECT_H

#include <vulkan/vulkan.hpp>

namespace ZEROengine {
    struct VulkanGraphicsPipelineObject {
        VkPipeline vk_pipeline;
        VkPipelineLayout vk_pipeline_layout;
    };
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKAN_GRAPHICS_PIPELINE_OBJECT_H