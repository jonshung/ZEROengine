#ifndef ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H
#define ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H

#include <vk_mem_alloc.h>

#include "zeroengine_vulkan/VulkanContext.hpp"
#include "zeroengine_vulkan/VulkanBasicScreenRenderer.hpp"
#include "zeroengine_vulkan/VulkanRenderContext.hpp"
#include "zeroengine_vulkan/VulkanBuffer.hpp"
#include "zeroengine_vulkan/VulkanWindowContext.hpp"
#include "zeroengine_core/GraphicalModule.hpp"

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
 * - Vertices buffer.
 * 
 */
class VulkanGraphicalModule : public GraphicalModule {
private:
    VulkanContext vulkan_context;
    VkSurfaceKHR vk_surface;

    std::vector<std::size_t> frame_command_buffer_indices;
    VulkanRenderContext vulkan_render_context;
    VulkanBasicScreenRenderer vulkan_screen_renderer;

    VulkanGraphicsPipelineTemplate forwardpass_pipeline_template;
    std::unique_ptr<VulkanGraphicsPipelineBuffer> vk_graphics_pipeline_buffer;

    VmaAllocator vma_alloc;
    VulkanBuffer vertices_buffer;
    VulkanBuffer index_buffer;

private:
    void recordAndSubmitStagingCommandBuffer();

public:
    VulkanContext& getVulkanContext() {
        return this->vulkan_context;
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
    void initGraphicalModule() override;
    void drawFrame() override;
    void handleResize() override;
    
    void cleanup() override;
};

#endif // #ifndef ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H