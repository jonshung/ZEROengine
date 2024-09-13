#ifndef ZEROENGINE_VULKAN_BASIC_SCREEN_RENDERER_H
#define ZEROENGINE_VULKAN_BASIC_SCREEN_RENDERER_H

#include <vulkan/vulkan.hpp>

#include <vector>

#include "VulkanRenderer.hpp"
#include "VulkanPipelineBuffer.hpp"

/**
 * @brief The basic Renderer class for camera-view into the rendering scene.
 * 
 */
class VulkanBasicScreenRenderer : public VulkanRenderer {
public:
    //virtual void cleanup(VkDevice &device) override;  not used at the moment

    virtual void draw(VkCommandBuffer &recording_buffer, VulkanGraphicsPipelineBuffer *const g_pipeline_buffer) override;
};

#endif // #ifndef VULKAN_BASIC_SCREEN_RENDERER_H