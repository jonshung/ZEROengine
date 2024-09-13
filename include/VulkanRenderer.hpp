#ifndef ZEROENGINE_VULKAN_RENDERER_H
#define ZEROENGINE_VULKAN_RENDERER_H

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <vector>
#include <functional>

#include "VulkanRenderer_define.hpp"
#include "VulkanRenderTarget.hpp"
#include "VulkanPipelineBuffer.hpp"

/**
 * @brief General purpose Renderer.
 * A Renderer implements all necessary logics to record secondary command buffers to
 * write to a specific VulkanRenderTarget.
 * 
 */
class VulkanRenderer {
private:
    VulkanRendererSettings settings;

// initialization and cleanup procedures
public:
    /**
     * @brief Before usage, any Renderer should be initialized by calling this function.
     * 
     * @param settings 
     */
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

    virtual void reset(VkCommandBuffer &recording_buffer);
    virtual void begin(VkCommandBuffer &recording_buffer, VulkanRenderTarget &render_target);
    virtual void configureViewportAndScissor(VkCommandBuffer &recording_buffer, VkExtent2D &extent);
    // currently directly passing the graphics pipeline buffer to every renderer via here. Should be passing a structure of
    // world objects material and geometry informations in the future
    virtual void draw(VkCommandBuffer &recording_buffer, VulkanGraphicsPipelineBuffer *const g_pipeline_buffer) = 0;
    virtual void end(VkCommandBuffer &recording_buffer);
    void record(VkCommandBuffer &recording_buffer, VulkanRenderTarget &render_target, VulkanGraphicsPipelineBuffer *const);

}; // class VulkanRenderer

#endif // #ifndef VULKAN_RENDERER_H