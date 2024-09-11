#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <memory>
#include <vector>
#include <functional>

#include "VulkanRenderer_def.hpp"
#include "VulkanPipelineBuffer.hpp"
#include "VulkanRenderTarget.hpp"

struct VulkanSecondaryCommandBuffer {
    bool ready;
    VkCommandBuffer buffer;
};

class VulkanRenderContext {
private:
    VkDevice vk_device_handle;
    VkQueueInfo vk_graphical_queue;

// public synchronization api for application draw call
public:
    void initVulkanRenderContext(const VulkanRenderContextCreateInfo &parameters);
    
    void begin(const uint32_t &target_index);
    void recordRenderPassCommandBuffer(const uint32_t &target_index, VulkanRenderTarget &render_target);
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
    std::vector<VulkanSecondaryCommandBuffer> vk_secondary_cmd_buffers;

    // create the process's command pool
    void createCommandPool(uint32_t queueFamilyIndex);

public:
    // allocate additional command buffers
    std::vector<size_t> requestCommandBufferAllocation(uint32_t count = 1);
    int32_t getCommandBuffer(size_t index, VulkanSecondaryCommandBuffer **buffer) {
        if(index >= this->vk_secondary_cmd_buffers.size()) {
            return -1;
        }
        *buffer = &this->vk_secondary_cmd_buffers[index];
        return 0;
    }
}; // class VulkanRenderContext

class VulkanRenderer {
private:
    VulkanRendererSettings settings;

// initialization and cleanup procedures
public:
    void initVulkanRenderer(const VulkanRendererSettings &settings);
    virtual void cleanup(VkDevice &device) {
        (void) device;
    }

public:
    void setVerticesBuffer();
    void reloadSettings(const VulkanRendererSettings &_settings) {
        this->settings = _settings;
    }
    VulkanRendererSettings getSettings() {
        return this->settings;
    }

    void reset(VkCommandBuffer &recording_buffer);
    virtual void begin(VkCommandBuffer &recording_buffer, VulkanRenderTarget &render_target);
    virtual void configureViewportAndScissor(VkCommandBuffer &recording_buffer, VkExtent2D &extent);
    virtual void draw(VkCommandBuffer &recording_buffer) = 0;
    virtual void end(VkCommandBuffer &recording_buffer);
    virtual void record(VkCommandBuffer &recording_buffer, VulkanRenderTarget &render_target);

}; // class VulkanRenderer

#endif // #ifndef VULKAN_RENDERER_H