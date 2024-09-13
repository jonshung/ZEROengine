#ifndef ZEROENGINE_VULKAN_BASIC_SCREEN_RENDERER_H
#define ZEROENGINE_VULKAN_BASIC_SCREEN_RENDERER_H

#include <vulkan/vulkan.hpp>

#include <vector>
#include <memory>

#include "VulkanContext.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanRenderer.hpp"
#include "VulkanPipelineBuffer.hpp"
#include "Vulkan_define.hpp"
/**
 * @brief The basic Renderer class for camera-view into the rendering scene.
 * 
 */
class VulkanBasicScreenRenderer : public VulkanRenderer {
// Command buffers
private:
    std::vector<VulkanSecondaryCommandBuffer> frame_cmd_buffers;
    const uint32_t max_queued_frame = 2;

    void allocateSecondaryCommandBuffer(const uint32_t &count);


// Swapchain and RenderTarget
private:
    VkSurfaceFormatKHR selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
    VkPresentModeKHR selectSwapChainPresentationMode(const std::vector<VkPresentModeKHR> &modes);
    VkExtent2D selectSwapChainExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkSwapchainKHR vk_swapchain;
    std::shared_ptr<VkRenderPass> vk_swapchain_render_pass;
    std::vector<VulkanRenderTarget> vk_swapchain_render_targets;
    bool enable_depth_stencil_subpass = false;
    
    VkFormat vk_swapchain_image_format;
    VkExtent2D vk_swapchain_extent;

// public swapchain related API
public:
    std::vector<VulkanRenderTarget>& requestSwapChainRenderTargets() {
        return this->vk_swapchain_render_targets;
    }
    VkRenderPass& requestSwapChainRenderPass() {
        return (*this->vk_swapchain_render_pass);
    }
    VkResult acquireSwapChainImageIndex(uint32_t &index, VkSemaphore semaphore_lock);
    void presentImage(VkSemaphore semaphore_lock);
    const uint32_t& getCurrentFrameIndex() const;
    VkResult queryAcquireAndStoreFrame(VkSemaphore &lock) {
        return this->acquireSwapChainImageIndex(this->swapchain_acquired_image, lock);
    }
    void queueNextFrame();
    void handleResize();
    uint32_t getMaxQueuedFrame() const {
        return this->max_queued_frame;
    }

    uint32_t current_frame_index = 0;
    uint32_t swapchain_acquired_image;

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

// Initialization and cleanup procedures
public:
    VulkanBasicScreenRenderer() : VulkanRenderer() {}
    virtual void initVulkanRenderer(const VulkanRendererSettings &settings, VulkanContext &vulkan_context, const uint32_t &queue_family_index) override;
    void initSwapChain();
    void initSwapChainRenderPass();
    void initSwapChainRenderTargets();
    
    void reload_swapChain();
    void cleanup_swapChain();
    void cleanup_concurrency_locks();
    virtual void cleanup() override;

public:
    virtual void reset() override;
    virtual void begin() override;
    virtual void configureViewportAndScissor(VkExtent2D &extent) override;
    // currently directly passing the graphics pipeline buffer to every renderer via here. Should be passing a structure of
    // world objects material and geometry informations in the future
    virtual void draw(VulkanGraphicsPipelineBuffer *const g_pipeline_buffer) override;
    virtual void end() override;
    virtual std::vector<std::pair<VulkanRenderTarget*, VulkanSecondaryCommandBuffer>> record(VulkanGraphicsPipelineBuffer *const pipeline_buffer) override;

    void setVerticesBuffer() {

    }
};

#endif // #ifndef VULKAN_BASIC_SCREEN_RENDERER_H