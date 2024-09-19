#ifndef ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H
#define ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H

#include <vk_mem_alloc.h>

#include "zeroengine_vulkan/VulkanContext.hpp"
#include "zeroengine_vulkan/VulkanBasicScreenRenderer.hpp"
#include "zeroengine_vulkan/VulkanRenderContext.hpp"
#include "zeroengine_vulkan/VulkanBuffer.hpp"
#include "zeroengine_vulkan/VulkanWindowContext.hpp"
#include "zeroengine_core/GraphicalModule.hpp"

namespace ZEROengine {
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

        VulkanRenderContext vulkan_render_context;
        VulkanBasicScreenRenderer vulkan_screen_renderer;

        VulkanGraphicsPipelineTemplate forwardpass_pipeline_template;
        VulkanGraphicsPipelineBuffer vk_graphics_pipeline_buffer;

        VmaAllocator vma_alloc;
        VulkanBuffer vertices_buffer;
        VulkanBuffer index_buffer;

        VkDescriptorPool vk_descriptor_pool;
        std::vector<VkDescriptorSet> vk_descriptor_sets;
        std::vector<VulkanBuffer> uniform_buffers;

    private:
        void recordAndSubmitStagingCommandBuffer();

    public:
        ZEROResult getVulkanContext(VulkanContext **ret) {
            *ret = &this->vulkan_context;
            return { ZERO_SUCCESS, "" };
        }
        ZEROResult getRenderContext(VulkanRenderContext **ret) {
            *ret = &this->vulkan_render_context;
            return { ZERO_SUCCESS, "" };
        }
        ZEROResult getScreenRenderer(VulkanBasicScreenRenderer **ret) {
            *ret = &this->vulkan_screen_renderer;
            return { ZERO_SUCCESS, "" };
        }
        ZEROResult getForwardPassPipelineTemplate(VulkanGraphicsPipelineTemplate **ret) {
            *ret = &this->forwardpass_pipeline_template;
            return { ZERO_SUCCESS, "" };
        }
        ZEROResult getGraphicsPipelineBuffer(VulkanGraphicsPipelineBuffer **ret) {
            *ret = &this->vk_graphics_pipeline_buffer;
            return { ZERO_SUCCESS, "" };
        }
        void initGraphicalModule() override;
        void drawFrame() override;
        void handleResize() override;
        
        void cleanup() override;
    }; // class VulkanGraphicalModule

} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H