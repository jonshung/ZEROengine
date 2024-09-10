#include "VulkanRenderer.hpp"
#include "vulkan/vk_enum_string_helper.h"

#include <array>

void VulkanRenderer::initVulkanRenderer(const VulkanRendererCreateInfo *info) {
    this->vk_device_handle = info->device;
    this->graphical_queue = info->queue_info;
    
    this->reloadDependencies(info->dependencies);
    this->reloadSettings(info->settings);
    this->reloadRenderPass();
    this->reloadFramebuffers();

    createCommandPool(info->queue_info.queueFamilyIndex);
    createCommandBuffer(info->dependencies.image_views.size());
    createConcurrencyLock(info->dependencies.image_views.size()); // locks for each frame
}

void VulkanRenderer::reloadRenderPass() {
    VkDevice& device = this->vk_device_handle;

    std::array<VkAttachmentDescription, 2> attachments = {};
    attachments[0].format = this->dependencies.framebuffer_format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.colorAttachmentCount = 1;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pColorAttachments = &color_attachment_ref;

	std::array<VkSubpassDependency, 2> subpass_dep = {};
    subpass_dep[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_dep[0].dstSubpass = 0;
	subpass_dep[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpass_dep[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dep[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    subpass_dep[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpass_dep[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    /* future support
    if(this->settings.enableDepthStencil) {
        attachments[1].format = VK_FORMAT_D16_UNORM;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attachment_ref = {};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        subpass.pDepthStencilAttachment = &depth_attachment_ref;
        subpass_dep[1].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dep[1].dstSubpass = 0;
        subpass_dep[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dep[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpass_dep[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpass_dep[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpass_dep[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    }
    */

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>(this->settings.enableDepthStencil ? 2 : 1);
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = static_cast<uint32_t>(this->settings.enableDepthStencil ? 2 : 1);
    render_pass_info.pDependencies = subpass_dep.data();
    VkResult rslt = vkCreateRenderPass(device, &render_pass_info, nullptr, &this->vk_render_pass);
    if (rslt != VK_SUCCESS) {
        throw std::runtime_error("vkCreateRenderPass failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderer::reloadFramebuffers() {
    VkDevice& device = this->vk_device_handle;

    this->vk_framebuffers.resize(this->dependencies.image_views.size());
    for(size_t i = 0; i < static_cast<size_t>(this->vk_framebuffers.size()); ++i) {
        VkImageView* img_view_ref = &this->dependencies.image_views[i];
        VkFramebufferCreateInfo framebuffers_create_info{};
        framebuffers_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffers_create_info.width = this->dependencies.extent.width;
        framebuffers_create_info.height = this->dependencies.extent.height;
        framebuffers_create_info.layers = 1;
        framebuffers_create_info.attachmentCount = 1;
        framebuffers_create_info.pAttachments = img_view_ref;
        framebuffers_create_info.renderPass = this->vk_render_pass;
        VkResult rslt = vkCreateFramebuffer(device, &framebuffers_create_info, nullptr, &this->vk_framebuffers[i]);
        if(rslt != VK_SUCCESS) {
            throw std::runtime_error("vkCreateFramebuffer() failed, err: + " + std::string(string_VkResult(rslt)));
        }
    }
}

void VulkanRenderer::createCommandPool(uint32_t queueFamilyIndex) {
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

void VulkanRenderer::createCommandBuffer(uint32_t count) {
    VkDevice& device = this->vk_device_handle;

    this->vk_cmd_buffers.resize(this->vk_cmd_buffers.size() + count);
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = this->vk_cmd_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = count;
    VkResult rslt = vkAllocateCommandBuffers(device, &alloc_info, this->vk_cmd_buffers.data());
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkAllocateCommandBuffers() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderer::beginRenderPassCommandBuffer(const uint32_t &frame_cmd_buffer_index, uint32_t image_index) {
    if(!this->framebuffer_validation) return;
    VkCommandBufferBeginInfo cmd_buffer_begin{};
    cmd_buffer_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buffer_begin.pInheritanceInfo = nullptr;
    cmd_buffer_begin.flags = 0;
    VkCommandBuffer &cmd_buffer = this->vk_cmd_buffers[frame_cmd_buffer_index];
    VkResult rslt = vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_begin);
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkBeginCommandBuffer() failed, err: " + std::string(string_VkResult(rslt)));
    }

    VkRenderPassBeginInfo render_pass_begin{};
    render_pass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin.framebuffer = this->vk_framebuffers[image_index];
    render_pass_begin.renderPass = this->vk_render_pass;
    render_pass_begin.renderArea.extent = this->dependencies.extent;
    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_begin.clearValueCount = 1;
    render_pass_begin.pClearValues = &clear_color;
    vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderer::recordPipelineRenderCommandBuffer(const uint32_t &frame_cmd_buffer_index, VkPipeline pipeline) {
    if(!this->framebuffer_validation) return;
    VkCommandBuffer &cmd_buffer = this->vk_cmd_buffers[frame_cmd_buffer_index];
    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(this->dependencies.extent.width);
    viewport.height = static_cast<float>(this->dependencies.extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = this->dependencies.extent;
    vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);
    vkCmdDraw(cmd_buffer, 3, 1, 0 ,0);
}

void VulkanRenderer::endRenderPassCommandBuffer(const uint32_t &frame_cmd_buffer_index) {
    if(!this->framebuffer_validation) return;
    VkCommandBuffer &cmd_buffer = this->vk_cmd_buffers[frame_cmd_buffer_index];
    vkCmdEndRenderPass(cmd_buffer);
    VkResult rslt;
    if ((rslt = vkEndCommandBuffer(cmd_buffer)) != VK_SUCCESS) {
        throw std::runtime_error("vkEndCommandBuffer() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderer::submitRenderCommandBuffer(const uint32_t &frame_cmd_buffer_index) {
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &this->vk_cmd_buffers[frame_cmd_buffer_index];
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &this->vk_image_mutex[frame_cmd_buffer_index];
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &this->vk_rendering_mutex[frame_cmd_buffer_index];
    VkResult rslt = vkQueueSubmit(this->graphical_queue.queue, 1, &submit_info, this->vk_presentation_mutex[frame_cmd_buffer_index]);
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("submitRenderThreadCommandBuffer()::vkQueueSubmit() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderer::resetRenderCommandBuffer(const uint32_t &frame_cmd_buffer_index) {
    vkResetCommandBuffer(this->vk_cmd_buffers[frame_cmd_buffer_index], 0);
}

void VulkanRenderer::createConcurrencyLock(uint32_t count) {
    VkDevice& device = this->vk_device_handle;

    this->vk_image_mutex.resize(this->vk_image_mutex.size() + count);
    this->vk_rendering_mutex.resize(this->vk_rendering_mutex.size() + count);
    this->vk_presentation_mutex.resize(this->vk_presentation_mutex.size() + count);

    VkSemaphoreCreateInfo semaphore_create{};
    semaphore_create.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult rslt;
    for(uint32_t i = 0; i < count; ++i) {
        rslt = vkCreateSemaphore(device, &semaphore_create, nullptr, &this->vk_image_mutex[i]);
        if(rslt != VK_SUCCESS) {
            throw std::runtime_error("vkCreateSemaphore() failed, err: " + std::string(string_VkResult(rslt)));
        }
        rslt = vkCreateSemaphore(device, &semaphore_create, nullptr, &this->vk_rendering_mutex[i]);
        if(rslt != VK_SUCCESS) {
            throw std::runtime_error("vkCreateSemaphore() failed, err: " + std::string(string_VkResult(rslt)));
        }
        VkFenceCreateInfo fence_create{};
        fence_create.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        rslt = vkCreateFence(device, &fence_create, nullptr, &this->vk_presentation_mutex[i]);
        if(rslt != VK_SUCCESS) {
            throw std::runtime_error("vkCreateFence() failed, err: " + std::string(string_VkResult(rslt)));
        }
    }
}

void VulkanRenderer::setVerticesBuffer() {
    // TODO
}

void VulkanRenderer::cleanup_concurrency_locks() {
    VkDevice& device = this->vk_device_handle;

    // concurrency locks count should be synchronized
    uint32_t buffer_count = this->vk_image_mutex.size();
    for(uint32_t i = 0; i < buffer_count; ++i) {
        vkDestroyFence(device, this->vk_presentation_mutex[i], nullptr);
        vkDestroySemaphore(device, this->vk_rendering_mutex[i], nullptr);
        vkDestroySemaphore(device, this->vk_image_mutex[i], nullptr);
    }
}

void VulkanRenderer::cleanup_commandBuffers() {
    VkDevice& device = this->vk_device_handle;
    vkDestroyCommandPool(device, this->vk_cmd_pool, nullptr);
}

void VulkanRenderer::cleanup_framebuffers() {
    VkDevice& device = this->vk_device_handle;
    for(auto &fb : this->vk_framebuffers) {
        vkDestroyFramebuffer(device, fb, nullptr);
    }
}

void VulkanRenderer::cleanup() {
    VkDevice& device = this->vk_device_handle;
    cleanup_concurrency_locks();
    cleanup_commandBuffers();
    cleanup_framebuffers();
    
    vkDestroyRenderPass(device, this->vk_render_pass, nullptr);
}