#ifndef ZEROENGINE_VULKAN_RENDERER_H
#define ZEROENGINE_VULKAN_RENDERER_H

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <vector>
#include <utility>
#include <functional>

#include "zeroengine_vulkan/VulkanContext.hpp"
#include "zeroengine_vulkan/VulkanRenderer_define.hpp"
#include "zeroengine_vulkan/VulkanRenderTarget.hpp"
#include "zeroengine_vulkan/VulkanPipelineBuffer.hpp"

/**
 * @brief General purpose Renderer abstraction.
 * A Renderer implements all necessary logics to record secondary command buffers to submit to a RenderContext for execution
 * 
 */
class VulkanRenderer {
private:
    VulkanRendererSettings settings;

protected:
    VkCommandPool vk_render_cmd_pool;
    VulkanContext* vulkan_context;

// initialization and cleanup procedures
public:
    /**
     * @brief Before usage, any Renderer should be initialized by calling this function.
     * 
     * @param settings 
     */
    virtual void initVulkanRenderer(const VulkanRendererSettings &settings, VulkanContext &vulkan_context, const uint32_t &queue_family_index);
    virtual void cleanup() {
        vkDestroyCommandPool(this->vulkan_context->getDevice(), this->vk_render_cmd_pool, nullptr);
    }

public:
    VulkanRenderer() : settings({}) {}

    void reloadSettings(const VulkanRendererSettings &_settings) {
        this->settings = _settings;
    }
    VulkanRendererSettings getSettings() {
        return this->settings;
    }

    virtual void reset() {
        vkResetCommandPool(this->vulkan_context->getDevice(), this->vk_render_cmd_pool, 0);
    }
    virtual void begin() = 0;
    virtual void configureViewportAndScissor(VkExtent2D &extent) = 0;
    // currently directly passing the graphics pipeline buffer to every renderer via here. Should be passing a structure of
    // world objects material and geometry informations in the future
    virtual void draw(VulkanGraphicsPipelineBuffer *const g_pipeline_buffer) = 0;
    virtual void end() = 0;
    virtual std::vector<std::pair<VulkanRenderTarget*, VulkanSecondaryCommandBuffer>> record(VulkanGraphicsPipelineBuffer *const) = 0;

}; // class VulkanRenderer

#endif // #ifndef VULKAN_RENDERER_H