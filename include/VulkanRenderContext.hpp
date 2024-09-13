#ifndef ZEROENGINE_VULKAN_RENDER_CONTEXT_H
#define ZEROENGINE_VULKAN_RENDER_CONTEXT_H

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <vector>

#include "VulkanRenderer_define.hpp"
#include "VulkanRenderTarget.hpp"
#include "Vulkan_define.hpp"

struct VulkanRenderContextCreateInfo {
    VkQueueInfo queue_info;
    VkDevice device;
    uint32_t submission_queue_count;
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

// public synchronization api for application draw call
public:
    void initVulkanRenderContext(const VulkanRenderContextCreateInfo &parameters);
    
    void begin(const uint32_t &target_index);
    void recordRenderPassCommandBuffer(const uint32_t &target_index, const VulkanSecondaryCommandBuffer &secondary_cmd_buffer, VulkanRenderTarget &render_target);
    void end(const uint32_t &target_index);
    void submit(const uint32_t &target_index, bool enable_wait_semaphore = false);
    void reset(const uint32_t &target_index);

    void cleanup_concurrency_locks();
    void cleanup_commandBuffers();
    void cleanup();

// concurrency synchronization locks
private:
    std::vector<VkSemaphore> vk_image_mutex; // mutexes use exclusively to swapchain at current moment
    std::vector<VkSemaphore> vk_rendering_mutex; // hardware rendering completion signal mutex
    std::vector<VkFence> vk_presentation_mutex; // host rendering completion signal mutex

public:
    VkFence getPresentationLock(const uint32_t &target_index) {
        return this->vk_presentation_mutex[target_index];
    }
    VkSemaphore getImageLock(const uint32_t &target_index) {
        return this->vk_image_mutex[target_index];
    }
    VkSemaphore getRenderingLock(const uint32_t &target_index) {
        return this->vk_rendering_mutex[target_index];
    }
    // allocate additional concurrency lock for synchronization
    void createConcurrencyLock(const uint32_t &target_index);

// command pool and buffers
private:
    VkCommandPool vk_cmd_pool;
    std::vector<VkCommandBuffer> vk_primary_cmd_buffer;

    void createCommandPool(const uint32_t &queueFamilyIndex);
}; // class VulkanRenderContext

#endif // #ifndef ZEROENGINE_VULKAN_RENDER_CONTEXT_H