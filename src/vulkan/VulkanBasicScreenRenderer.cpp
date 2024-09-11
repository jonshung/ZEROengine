#include "VulkanBasicScreenRenderer.hpp"

#include <vulkan/vk_enum_string_helper.h>

VulkanBasicScreenRenderer::VulkanBasicScreenRenderer() {
    this->vk_graphics_pipeline_buffer = std::make_unique<VulkanPipelineBuffer>();
}

void VulkanBasicScreenRenderer::draw(VkCommandBuffer &recording_buffer) {
    // testing purpose
    std::vector<VkPipeline>& pipelines = this->vk_graphics_pipeline_buffer.get()->getAllPipelines();
    if(pipelines.size() == 0) return;
    vkCmdBindPipeline(recording_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines[0]);
    vkCmdDraw(recording_buffer, 3, 1, 0 ,0);
}

void VulkanBasicScreenRenderer::cleanup(VkDevice &device) {
    (*this->vk_graphics_pipeline_buffer).cleanup(device);
}