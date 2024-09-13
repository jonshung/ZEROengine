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

// public synchronization api for application draw call
public:
    void initVulkanRenderContext(const VulkanRenderContextCreateInfo &parameters);
    
    void begin(const uint32_t &target_index);
    void recordRenderPassCommandBuffer(const uint32_t &target_index, const VulkanSecondaryCommandBuffer &secondary_cmd_buffer, VulkanRenderTarget &render_target);
    void end(const uint32_t &target_index);
    void submit(const uint32_t &target_index, VkSemaphore semaphore = VK_NULL_HANDLE, VkSemaphore signal_semaphore = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE);
    void reset(const uint32_t &target_index);

    void cleanup_commandBuffers();
    void cleanup();

// command pool and buffers
private:
    VkCommandPool vk_cmd_pool;
    std::vector<VkCommandBuffer> vk_primary_cmd_buffer;

    void createCommandPool(const uint32_t &queueFamilyIndex);
}; // class VulkanRenderContext

#endif // #ifndef ZEROENGINE_VULKAN_RENDER_CONTEXT_H