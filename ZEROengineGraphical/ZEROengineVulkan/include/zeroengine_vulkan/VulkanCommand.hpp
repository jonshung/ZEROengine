#ifndef ZEROENGINE_VULKAN_COMMAND_H
#define ZEROENGINE_VULKAN_COMMAND_H

#include "zeroengine_vulkan/Vulkan_define.hpp"

namespace ZEROengine {
    struct VulkanGraphicsRecordingInfo {
        VulkanSecondaryCommandBuffer cmd;
        VkRenderPass vk_renderpass;
        VkExtent2D vk_render_area;
        VkFramebuffer vk_framebuffer;
    };

    struct VulkanTransferRecordingInfo {
        VulkanSecondaryCommandBuffer cmd;
    };
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKAN_COMMAND_H