#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <memory>
#include <vector>
#include <functional>

#include "VulkanRenderer_def.hpp"
#include "VulkanPipelineBuffer.hpp"

class VulkanRenderer {
private:
    VulkanRendererSettings settings;
    bool framebuffer_validation = true;

// initialization and cleanup procedures
public:
    void initVulkanRenderer(const VulkanRendererCreateInfo *info);
    
    void cleanup_concurrency_locks(VkDevice device);
    void cleanup_commandBuffers(VkDevice device);
    void cleanup_framebuffers(VkDevice device);
    void cleanup(VkDevice device);

// public synchronization api for application draw call
public:
    void setVerticesBuffer();
    void reloadDependencies(VulkanRendererDependencies _dependencies) {
        this->dependencies = _dependencies;
    }
    void reloadSettings(VulkanRendererSettings _settings) {
        this->settings = _settings;
    }
    void reloadRenderPass(VkDevice device);
    void reloadFramebuffers(VkDevice device);
    void validateFramebuffer() {
        this->framebuffer_validation = true;
    }
    void invalidateFramebuffer() {
        this->framebuffer_validation = false;
    }

    VkFence getPresentationLock(uint32_t frame_index) {
        return this->vk_presentation_mutex[frame_index];
    }
    VkSemaphore getImageLock(uint32_t frame_index) {
        return this->vk_image_mutex[frame_index];
    }
    VkSemaphore getRenderingLock(uint32_t frame_index) {
        return this->vk_rendering_mutex[frame_index];
    }
    VkRenderPass getRenderPass() {
        return this->vk_render_pass;
    }

    void beginRenderPassCommandBuffer(const uint32_t &frame_cmd_buffer_index, uint32_t image_index);
    void recordPipelineRenderCommandBuffer(const uint32_t &frame_cmd_buffer_index, VkPipeline pipeline);
    void endRenderPassCommandBuffer(const uint32_t &frame_cmd_buffer_index);
    void resetRenderCommandBuffer(const uint32_t &frame_cmd_buffer_index);
    void submitRenderCommandBuffer(const uint32_t &frame_cmd_buffer_index);

// command pool and buffers
private:
    // create the process's command pool
    void createCommandPool(VkDevice device, uint32_t queueFamilyIndex);
    // allocate additional command buffers
    void createCommandBuffer(VkDevice device, uint32_t count);
    // allocate additional concurrency lock for synchronization
    void createConcurrencyLock(VkDevice device, uint32_t count);

private:
    VkCommandPool vk_cmd_pool;
    std::vector<VkCommandBuffer> vk_cmd_buffers;
    std::vector<VkSemaphore> vk_image_mutex;
    std::vector<VkSemaphore> vk_rendering_mutex;
    std::vector<VkFence> vk_presentation_mutex; // waiting for previous frame

private:
    VkRenderPass vk_render_pass;
    std::vector<VkFramebuffer> vk_framebuffers;

    VulkanRendererDependencies dependencies;
};

#endif // #ifndef VULKAN_RENDERER_H