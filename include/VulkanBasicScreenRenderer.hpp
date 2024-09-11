#ifndef VULKAN_BASIC_SCREEN_RENDERER_H
#define VULKAN_BASIC_SCREEN_RENDERER_H

#include <vulkan/vulkan.hpp>

#include <vector>

#include "VulkanRenderer.hpp"

class VulkanBasicScreenRenderer : public VulkanRenderer {
private:
    std::unique_ptr<VulkanPipelineBuffer> vk_graphics_pipeline_buffer;

public:
    VulkanBasicScreenRenderer();
    virtual void cleanup(VkDevice &device) override;

    virtual void draw(VkCommandBuffer &recording_buffer) override;

    VulkanPipelineBuffer& getGraphicsPipelineBuffer() {
        return *this->vk_graphics_pipeline_buffer;
    }
};

#endif // #ifndef VULKAN_BASIC_SCREEN_RENDERER_H