#include "VulkanBasicScreenRenderer.hpp"

#include <unordered_map>

#include <vulkan/vk_enum_string_helper.h>

void VulkanBasicScreenRenderer::draw(VkCommandBuffer &recording_buffer, VulkanGraphicsPipelineBuffer *g_pipeline_buffer) {
    // testing purpose
    if(!g_pipeline_buffer) return;
    std::unordered_map<std::size_t, VkPipeline> &pipelines = g_pipeline_buffer->getAllPipelines();
    if(pipelines.size() == 0) return;
    VkPipeline& testing_pipeline = pipelines.begin()->second;
    vkCmdBindPipeline(recording_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, testing_pipeline);
    vkCmdDraw(recording_buffer, 3, 1, 0 ,0);
}