#include "VulkanRenderer.hpp"

#include <numeric>

#include <vulkan/vk_enum_string_helper.h>

void VulkanRenderContext::initVulkanRenderContext(const VulkanRenderContextCreateInfo &parameters) {
    this->vk_device_handle = parameters.device;
    this->vk_graphical_queue = parameters.queue_info;
    
    createCommandPool(parameters.queue_info.queueFamilyIndex);
    this->vk_primary_cmd_buffer.resize(parameters.submission_queue_count);
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = this->vk_cmd_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = parameters.submission_queue_count;
    VkResult rslt = vkAllocateCommandBuffers(this->vk_device_handle, &alloc_info, this->vk_primary_cmd_buffer.data());
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkAllocateCommandBuffers() failed, err: " + std::string(string_VkResult(rslt)));
    }

    createConcurrencyLock(parameters.submission_queue_count); // locks for each frame
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

void VulkanRenderContext::recordRenderPassCommandBuffer(const uint32_t &target_index, VulkanRenderTarget &render_target) {
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

    std::vector<VkCommandBuffer> ready_buffers;
    for(auto &secondary_cmd_buffer : this->vk_secondary_cmd_buffers) {
        if(secondary_cmd_buffer.ready) ready_buffers.push_back(secondary_cmd_buffer.buffer);
    }
    vkCmdBeginRenderPass(primary_cmd_buffer, &render_pass_begin, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
    vkCmdExecuteCommands(primary_cmd_buffer, ready_buffers.size(), ready_buffers.data());
    vkCmdEndRenderPass(primary_cmd_buffer);
}

void VulkanRenderContext::end(const uint32_t &target_index) {
    VkResult rslt;
    if ((rslt = vkEndCommandBuffer(this->vk_primary_cmd_buffer[target_index])) != VK_SUCCESS) {
        throw std::runtime_error("vkEndCommandBuffer() failed, err: " + std::string(string_VkResult(rslt)));
    }
    for(auto &secondary_cmd_buffer : this->vk_secondary_cmd_buffers) {
        secondary_cmd_buffer.ready = false;
    }
}

void VulkanRenderContext::submit(const uint32_t &target_index, bool enable_wait_semaphore) {
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &this->vk_primary_cmd_buffer[target_index];
    if(enable_wait_semaphore) {
        VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.pWaitSemaphores = &this->vk_image_mutex[target_index];
    } else {
        submit_info.waitSemaphoreCount = 0;
        submit_info.pWaitDstStageMask = 0;
        submit_info.pWaitSemaphores = nullptr;
    }
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &this->vk_rendering_mutex[target_index];

    VkResult rslt = vkQueueSubmit(this->vk_graphical_queue.queue, 1, &submit_info, this->vk_presentation_mutex[target_index]);
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("submitRenderThreadCommandBuffer()::vkQueueSubmit() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderContext::reset(const uint32_t &target_index) {
    vkResetCommandBuffer(this->vk_primary_cmd_buffer[target_index], 0);
}

std::vector<size_t> VulkanRenderContext::requestCommandBufferAllocation(uint32_t count) {
    if(count == 0) return {};
    VkDevice& device = this->vk_device_handle;

    size_t emplace_index = this->vk_secondary_cmd_buffers.size();
    this->vk_secondary_cmd_buffers.resize(emplace_index + count);
    std::vector<VkCommandBuffer> cmd_buffer_emplace(this->vk_secondary_cmd_buffers.size());

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = this->vk_cmd_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    alloc_info.commandBufferCount = count;

    VkResult rslt = vkAllocateCommandBuffers(device, &alloc_info, cmd_buffer_emplace.data());
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkAllocateCommandBuffers() failed, err: " + std::string(string_VkResult(rslt)));
    }

    std::vector<size_t> buffers_ref(count);
    size_t ret_index = 0;
    for(size_t i = emplace_index; i < this->vk_secondary_cmd_buffers.size(); ++i) {
        this->vk_secondary_cmd_buffers[i].ready = false;
        this->vk_secondary_cmd_buffers[i].buffer = cmd_buffer_emplace[ret_index];
        buffers_ref[ret_index] = i;
        ++ret_index;
    }
    return buffers_ref;
}

void VulkanRenderContext::createCommandPool(uint32_t queueFamilyIndex) {
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

void VulkanRenderContext::createConcurrencyLock(const uint32_t &count) {
    VkDevice& device = this->vk_device_handle;

    uint32_t start_index = this->vk_image_mutex.size();
    this->vk_image_mutex.resize(start_index + count);
    this->vk_rendering_mutex.resize(start_index + count);
    this->vk_presentation_mutex.resize(start_index + count);

    VkSemaphoreCreateInfo semaphore_create{};
    semaphore_create.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult rslt;
    for(uint32_t i = start_index; i < this->vk_image_mutex.size(); ++i) {
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

void VulkanRenderContext::cleanup_concurrency_locks() {
    VkDevice& device = this->vk_device_handle;

    // concurrency locks count should be synchronized
    uint32_t buffer_count = this->vk_image_mutex.size();
    for(uint32_t i = 0; i < buffer_count; ++i) {
        vkDestroyFence(device, this->vk_presentation_mutex[i], nullptr);
        vkDestroySemaphore(device, this->vk_rendering_mutex[i], nullptr);
        vkDestroySemaphore(device, this->vk_image_mutex[i], nullptr);
    }
}

void VulkanRenderContext::cleanup_commandBuffers() {
    VkDevice& device = this->vk_device_handle;
    vkDestroyCommandPool(device, this->vk_cmd_pool, nullptr);
}

void VulkanRenderContext::cleanup() {
    cleanup_concurrency_locks();
    cleanup_commandBuffers();
}