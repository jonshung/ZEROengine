#ifndef ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H
#define ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H

#include "SDLWindowContext.hpp"
#include "VulkanContext.hpp"
#include "VulkanBasicScreenRenderer.hpp"
#include "VulkanRenderContext.hpp"

/**
 * @brief The engine graphical module. Providing necessary contexts and modules to draw to render targets, including the application window.
 * The module manages 
 * - the Vulkan Context,
 * - a Window Context, 
 * - a Vulkan Render Context, 
 * - a Vulkan Screen Renderer, 
 * - Command pools used for screen rendering
 * - A forward pass pipeline template for ease of access to pipeline creation.
 * - The general purpose Graphics Pipeline Buffer for all graphics-related pipeline of the engine
 * 
 */
class VulkanGraphicalModule {
private:
    SDLWindowContext window_context;
    VulkanContext vulkan_context;

    std::vector<std::size_t> frame_command_buffer_indices;
    VulkanRenderContext vulkan_render_context;
    VulkanBasicScreenRenderer vulkan_screen_renderer;
    
    VkCommandPool vk_draw_cmd_pool;
    std::vector<VulkanSecondaryCommandBuffer> vk_draw_cmd_buffers;

    VulkanGraphicsPipelineTemplate forwardpass_pipeline_template;
    std::unique_ptr<VulkanGraphicsPipelineBuffer> vk_graphics_pipeline_buffer;

public:
    VulkanContext& getVulkanContext() {
        return this->vulkan_context;
    }
    SDLWindowContext& getWindowContext() {
        return this->window_context;
    }
    VulkanRenderContext& getRenderContext() {
        return this->vulkan_render_context;
    }
    VulkanBasicScreenRenderer& getScreenRenderer() {
        return this->vulkan_screen_renderer;
    }
    VulkanGraphicsPipelineTemplate& getForwardPassPipelineTemplate() {
        return this->forwardpass_pipeline_template;
    }
    VulkanGraphicsPipelineBuffer& getGraphicsPipelineBuffer() {
        return *this->vk_graphics_pipeline_buffer;
    }
    void initVulkanGraphicalModule();
    void drawFrame();
    void handleResize();

    void requestCommandBufferAllocation(const uint32_t &count);
    void cleanup();
};

#endif // #ifndef ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H