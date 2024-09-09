#ifndef VULKAN_RENDERER_H
#define VULKAN_RENDERER_H

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <memory>
#include <vector>
#include <functional>

#include "VulkanRenderer_def.hpp"

// debug dynamic viewport state
const std::vector<VkDynamicState> dynamic_states = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

class VulkanRenderer {
// initialization and cleanup procedures
public:
    void initVulkanRenderer(const VulkanRendererCreateInfo *info);
    
    void cleanup_concurrency_locks(VkDevice device);
    void cleanup_commandBuffers(VkDevice device);
    void cleanup_pipelines(VkDevice device);
    void cleanup(VkDevice device);

// public synchronization api for application draw call
public:
    void setVerticesBuffer();
    void reloadDependencies(VulkanRendererDependencies new_reference);
    VkFence getPresentationLock(uint32_t frame_index);
    VkSemaphore getImageLock(uint32_t frame_index);
    VkSemaphore getRenderingLock(uint32_t frame_index);

    void recordRenderCommandBuffer(const uint32_t &frame_cmd_buffer_index, uint32_t image_index);
    void resetRenderCommandBuffer(const uint32_t &frame_cmd_buffer_index);
    void submitRenderCommandBuffer(const uint32_t &frame_cmd_buffer_index);
    void createGraphicsPipelines(VkDevice device, std::vector<ShaderData> shaders);

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

    VkQueue vk_graphics_queue;

// framebuffers
private:
    VkShaderModule createShaderModule(VkDevice device, const char* data, const size_t &data_size);

private:
    VkPipelineLayout vk_pipeline_layout;
    std::vector<VkPipeline> vk_graphics_pipelines;

    std::vector<VkFramebuffer> *vk_framebuffers;
    VkRenderPass vk_render_pass;
    VkExtent2D vk_extent;
};

#endif // #ifndef VULKAN_RENDERER_H