#ifndef ZEROENGINE_VULKAN_COMMAND_H
#define ZEROENGINE_VULKAN_COMMAND_H

#include "zeroengine_vulkan/VulkanDefines.hpp"

#include <vector>

namespace ZEROengine {
    struct VulkanCommandRecordingInfo {
        std::vector<VkCommandBuffer> cmd;
        std::vector<VkSemaphore> wait_semaphores;
        std::vector<VkPipelineStageFlags> wait_stages;

        std::vector<VkSemaphore> signal_semaphores;
        VkFence host_signal_fence;
        
    };
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKAN_COMMAND_H