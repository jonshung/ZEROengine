#ifndef ZEROENGINE_VULKAN_BASIC_SCREEN_RENDERER_H
#define ZEROENGINE_VULKAN_BASIC_SCREEN_RENDERER_H

#include <vulkan/vulkan.hpp>

#include <vector>
#include <memory>

#include "zeroengine_vulkan/VulkanContext.hpp"
#include "zeroengine_vulkan/VulkanRenderTarget.hpp"
#include "zeroengine_vulkan/VulkanRenderer.hpp"
#include "zeroengine_vulkan/VulkanPipelineBuffer.hpp"
#include "zeroengine_vulkan/Vulkan_define.hpp"
#include "zeroengine_core/WindowContext.hpp"

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

    uint32_t frame_width;
    uint32_t frame_height;
    VkSurfaceKHR vk_surface;
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
    void handleResize(const uint32_t &new_frame_width, const uint32_t &new_frame_height);
    void setFrameDimensions(const uint32_t &new_frame_width, const uint32_t &new_frame_height) {
        this->frame_width = new_frame_width;
        this->frame_height = new_frame_height;
    }
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

// Frame data
private:
    std::vector<BaseVertex> *vertices_data;
    std::vector<VkBuffer> vertices_buffer_handles;

    std::vector<uint32_t> *index_data;
    VkBuffer index_buffer_handle;

public:
    void setVerticesBufferHandle(const std::vector<VkBuffer> &buffer_handles) {
        this->vertices_buffer_handles = buffer_handles;
    }
    void setVerticesData(std::vector<BaseVertex>* const &p_vertices_data) {
        this->vertices_data = p_vertices_data;
    }
    void setIndexBufferHandle(const VkBuffer &buffer_handle) {
        this->index_buffer_handle = buffer_handle;
    }
    void setIndexData(std::vector<uint32_t>* const &p_index_data) {
        this->index_data = p_index_data;
    }
    
// Initialization and cleanup procedures
public:
    virtual void initVulkanRenderer(const VulkanRendererSettings &settings, VulkanContext &vulkan_context, const uint32_t &queue_family_index) override;
    void bindSurface(VkSurfaceKHR &surface) {
        this->vk_surface = surface;
    }
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
};

#endif // #ifndef VULKAN_BASIC_SCREEN_RENDERER_H