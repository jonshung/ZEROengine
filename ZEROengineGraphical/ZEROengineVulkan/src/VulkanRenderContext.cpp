#include "zeroengine_vulkan/VulkanRenderContext.hpp"

#include <numeric>

#include <vulkan/vk_enum_string_helper.h>

/*



                    TODO : MULTITHREAD LOCKING

*/




namespace ZEROengine {
    VulkanRenderContext::VulkanRenderContext() :
    vk_device_handle{},
    vk_graphical_queue{},
    vk_presentation_queue{},
    pending_recording()
    {}

    void VulkanRenderContext::initVulkanRenderContext(const VulkanRenderContextCreateInfo &parameters) {
        this->vk_device_handle = parameters.device;
        this->vk_graphical_queue = parameters.graphical_queue_info;
        this->vk_presentation_queue = parameters.presentation_queue_info;
        
        /*
        createCommandPool(parameters.graphical_queue_info.queueFamilyIndex);
        this->vk_primary_cmd_buffer.resize(parameters.primary_buffer_count);
        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = this->vk_cmd_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = parameters.primary_buffer_count;
        ZERO_VK_CHECK_EXCEPT(vkAllocateCommandBuffers(this->vk_device_handle, &alloc_info, this->vk_primary_cmd_buffer.data()));
        */
    }

    void VulkanRenderContext::queueCommandRecording(const VulkanCommandRecordingInfo &recording_info) {
        this->pending_recording.push_back(recording_info);
        // TODO: multithreading locks
    }

    void VulkanRenderContext::submit() {
        if(this->pending_recording.size() == 0) {
            return;
        }
        for(VulkanCommandRecordingInfo &recording_info : this->pending_recording) {
            VkSubmitInfo submit_info{};
            submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submit_info.commandBufferCount = static_cast<uint32_t>(recording_info.cmd.size());
            submit_info.pCommandBuffers = recording_info.cmd.data();

            submit_info.waitSemaphoreCount = static_cast<uint32_t>(recording_info.wait_semaphores.size());
            submit_info.pWaitSemaphores = recording_info.wait_semaphores.data();
            submit_info.pWaitDstStageMask = recording_info.wait_stages.data();

            submit_info.signalSemaphoreCount = static_cast<uint32_t>(recording_info.signal_semaphores.size());
            submit_info.pSignalSemaphores = recording_info.signal_semaphores.data();
            ZERO_VK_CHECK_EXCEPT(vkQueueSubmit(this->vk_graphical_queue.queue, 1, &submit_info, recording_info.host_signal_fence));
        }
        this->pending_recording.clear();

        for(VkPresentInfoKHR &present_info : this->pending_window_present) {
            vkQueuePresentKHR(this->vk_presentation_queue.queue, &present_info);
        }
        this->pending_window_present.clear();
    }

    void VulkanRenderContext::queuePresent(const VkPresentInfoKHR &present_info) {
        this->pending_window_present.push_back(present_info);
    }

    void VulkanRenderContext::cleanup() {
        // None
    }
} // namespace ZEROengine