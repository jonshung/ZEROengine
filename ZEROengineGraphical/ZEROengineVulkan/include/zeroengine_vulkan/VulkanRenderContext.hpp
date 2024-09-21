#ifndef ZEROENGINE_VULKAN_RENDER_CONTEXT_H
#define ZEROENGINE_VULKAN_RENDER_CONTEXT_H

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <vector>

#include "zeroengine_vulkan/VulkanCommand.hpp"
#include "zeroengine_vulkan/VulkanWindow.hpp"

namespace ZEROengine {
    struct VulkanRenderContextCreateInfo {
        VkQueueInfo graphical_queue_info;
        VkQueueInfo presentation_queue_info;
        VkDevice device;
        uint32_t primary_buffer_count;
    };

    /**
     * @brief A RenderContext is an opaque object managing all rendering state and is responsible for submitting the work to GPU.
     * All secondary command buffer must be recorded independently before being recorded into RenderContext's primary command buffer.
     * The application is responsible for managing its lifetime and cleanup procedures.
     */
    class VulkanRenderContext {
    private:
        VkDevice vk_device_handle;
        VkQueueInfo vk_graphical_queue;
        VkQueueInfo vk_presentation_queue;

    // command buffers and synchronization locks
    private:
        std::vector<VulkanCommandRecordingInfo> pending_recording;
    
    private:
        std::vector<VkPresentInfoKHR> pending_window_present;

    // public synchronization api for application draw call
    public:
        VulkanRenderContext();
        void initVulkanRenderContext(const VulkanRenderContextCreateInfo &parameters);
        
        void queueCommandRecording(const VulkanCommandRecordingInfo &recording_info);
        void queuePresent(const VkPresentInfoKHR &presenting_info);

        void submit();

        void waitOnGraphicalQueue() {
            vkQueueWaitIdle(this->vk_graphical_queue.queue);
        }

        void cleanup();
    }; // class VulkanRenderContext
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKAN_RENDER_CONTEXT_H