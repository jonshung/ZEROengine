#include "VulkanRenderContext.hpp"

#include <numeric>

#include <vulkan/vk_enum_string_helper.h>

void VulkanRenderContext::initVulkanRenderContext(const VulkanRenderContextCreateInfo &parameters) {
    this->vk_device_handle = parameters.device;
    this->vk_graphical_queue = parameters.queue_info;
    
    createCommandPool(parameters.queue_info.queueFamilyIndex);
    this->vk_primary_cmd_buffer.resize(parameters.primary_buffer_count);
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = this->vk_cmd_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = parameters.primary_buffer_count;
    VkResult rslt = vkAllocateCommandBuffers(this->vk_device_handle, &alloc_info, this->vk_primary_cmd_buffer.data());
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkAllocateCommandBuffers() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderContext::begin(const uint32_t &target_index) {
    VkCommandBufferBeginInfo cmd_buffer_begin{};
    cmd_buffer_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buffer_begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    cmd_buffer_begin.pInheritanceInfo = nullptr;
    cmd_buffer_begin.pNext = nullptr;

    VkResult rslt = vkBeginCommandBuffer(this->vk_primary_cmd_buffer[target_index], &cmd_buffer_begin);
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkBeginCommandBuffer() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderContext::recordRenderPassCommandBuffer(
    const uint32_t &target_index, 
    const VulkanSecondaryCommandBuffer &secondary_cmd_buffer,
    VulkanRenderTarget &render_target
) {
    // resetting previous command buffer recordings, then execute them.
    // TODO: creating a wrapper structure for secondary command buffer, allowing specification of reset mode.
    VkCommandBuffer& primary_cmd_buffer = this->vk_primary_cmd_buffer[target_index];

    VkRenderPassBeginInfo render_pass_begin{};
    render_pass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin.framebuffer = render_target.framebuffer;
    if(!render_target.render_pass.get()) {
        throw std::runtime_error("VulkanBasicScreenRenderer::begin() failed, err: Render Target's render pass is NULL");
    }
    render_pass_begin.renderPass = (*render_target.render_pass);
    render_pass_begin.renderArea.extent = render_target.framebuffer_extent;
    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_begin.clearValueCount = 1;
    render_pass_begin.pClearValues = &clear_color;
    
    vkCmdBeginRenderPass(primary_cmd_buffer, &render_pass_begin, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vkCmdExecuteCommands(primary_cmd_buffer, 1, &secondary_cmd_buffer.buffer);
    vkCmdEndRenderPass(primary_cmd_buffer);
}

void VulkanRenderContext::end(const uint32_t &target_index) {
    VkResult rslt;
    if ((rslt = vkEndCommandBuffer(this->vk_primary_cmd_buffer[target_index])) != VK_SUCCESS) {
        throw std::runtime_error("vkEndCommandBuffer() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderContext::submit(const uint32_t &target_index, VkSemaphore wait_semaphore, VkSemaphore signal_semaphore, VkFence fence) {
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &this->vk_primary_cmd_buffer[target_index];
    if(wait_semaphore != VK_NULL_HANDLE) {
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.pWaitSemaphores = &wait_semaphore;
    } else {
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitDstStageMask = 0;
        submit_info.pWaitSemaphores = nullptr;
    }
    if(signal_semaphore != VK_NULL_HANDLE) {
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &signal_semaphore;
    }

    VkResult rslt = vkQueueSubmit(this->vk_graphical_queue.queue, 1, &submit_info, fence);
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("submitRenderThreadCommandBuffer()::vkQueueSubmit() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderContext::reset(const uint32_t &target_index) {
    vkResetCommandBuffer(this->vk_primary_cmd_buffer[target_index], 0);
}

void VulkanRenderContext::createCommandPool(const uint32_t &queueFamilyIndex) {
    VkDevice& device = this->vk_device_handle;

    VkCommandPoolCreateInfo cmd_pool_create_info{};
    cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_create_info.queueFamilyIndex = queueFamilyIndex;
    VkResult rslt = vkCreateCommandPool(device, &cmd_pool_create_info, nullptr, &this->vk_cmd_pool);
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkCreateCommandPool() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderContext::cleanup_commandBuffers() {
    VkDevice& device = this->vk_device_handle;
    vkDestroyCommandPool(device, this->vk_cmd_pool, nullptr);
}

void VulkanRenderContext::cleanup() {
    cleanup_commandBuffers();
}