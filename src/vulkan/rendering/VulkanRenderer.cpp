#include "VulkanRenderer.hpp"
#include "vulkan/vk_enum_string_helper.h"

#include <array>

void VulkanRenderer::initVulkanRenderer(const VulkanRendererSettings &settings) {
    this->reloadSettings(settings);
}

void VulkanRenderer::reset(VkCommandBuffer &recording_buffer) {
    vkResetCommandBuffer(recording_buffer, 0);
}

void VulkanRenderer::begin(VkCommandBuffer &recording_buffer, VulkanRenderTarget &render_target) {
    VkCommandBufferBeginInfo cmd_buffer_begin{};
    cmd_buffer_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buffer_begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    VkCommandBufferInheritanceInfo inheritance_info{};
    inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritance_info.framebuffer = render_target.framebuffer;
    inheritance_info.renderPass = (*render_target.render_pass); // we specify our own render pass for each renderer
    inheritance_info.subpass = 0;
    cmd_buffer_begin.pInheritanceInfo = &inheritance_info;

    VkResult rslt = vkBeginCommandBuffer(recording_buffer, &cmd_buffer_begin);
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkBeginCommandBuffer() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderer::configureViewportAndScissor(VkCommandBuffer &recording_buffer, VkExtent2D &extent) {
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(recording_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(recording_buffer, 0, 1, &scissor);
}

void VulkanRenderer::end(VkCommandBuffer &recording_buffer) {
    VkResult rslt;
    if ((rslt = vkEndCommandBuffer(recording_buffer)) != VK_SUCCESS) {
        throw std::runtime_error("vkEndCommandBuffer() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderer::record(VkCommandBuffer &recording_buffer, VulkanRenderTarget &render_target, VulkanGraphicsPipelineBuffer *const pipeline_buffer) {
    this->reset(recording_buffer);
    this->begin(recording_buffer, render_target);
    this->configureViewportAndScissor(recording_buffer, render_target.framebuffer_extent);
    this->draw(recording_buffer, pipeline_buffer);
    this->end(recording_buffer);
}

void VulkanRenderer::setVerticesBuffer() {
    // TODO
}