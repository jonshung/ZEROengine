#include "zeroengine_vulkan/VulkanBasicScreenRenderer.hpp"

#include <unordered_map>

#include <vulkan/vk_enum_string_helper.h>

namespace ZEROengine {
    VulkanBasicScreenRenderer::VulkanBasicScreenRenderer() :
    VulkanRenderer(),
    current_frame_index{0},
    vertices_data{nullptr},
    vertices_buffer_handles{},
    index_data{nullptr},
    index_buffer_handle{},
    uniform_buffer_desc_set{},
    vk_image_mutex{},
    vk_rendering_mutex{},
    vk_presentation_mutex{}
    {}

    void VulkanBasicScreenRenderer::initVulkanRenderer(VulkanContext *vulkan_context) {
        VulkanRenderer::initVulkanRenderer(vulkan_context);
        allocateCommandBuffer(this->getMaxQueuedFrame());
        createSyncObjects(this->getMaxQueuedFrame());
    }


    void VulkanBasicScreenRenderer::bindRenderWindow(VulkanWindow *window) {
        this->render_window = window;
    }

    void VulkanBasicScreenRenderer::createSyncObjects(const uint32_t &count) {
        VkDevice device = this->vulkan_context->getDevice();
        
        uint32_t start_index = this->vk_image_mutex.size();
        this->vk_image_mutex.resize(start_index + count);
        this->vk_rendering_mutex.resize(start_index + count);
        this->vk_presentation_mutex.resize(start_index + count);

        VkSemaphoreCreateInfo semaphore_create{};
        semaphore_create.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        for(uint32_t i = start_index; i < this->vk_image_mutex.size(); ++i) {
            ZERO_VK_CHECK_EXCEPT(vkCreateSemaphore(device, &semaphore_create, nullptr, &this->vk_image_mutex[i]));
            ZERO_VK_CHECK_EXCEPT(vkCreateSemaphore(device, &semaphore_create, nullptr, &this->vk_rendering_mutex[i]));

            VkFenceCreateInfo fence_create{};
            fence_create.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fence_create.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            ZERO_VK_CHECK_EXCEPT(vkCreateFence(device, &fence_create, nullptr, &this->vk_presentation_mutex[i]));
        }
    }

    VkPresentInfoKHR VulkanBasicScreenRenderer::getPresentImageInfo() {
        VkSwapchainKHR *swapchain = this->render_window->getSwapchain();

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &this->vk_rendering_mutex[this->current_frame_index];
        present_info.swapchainCount = 1;
        present_info.pSwapchains = swapchain;
        present_info.pImageIndices = this->render_window->getAcquiredSwapchain();
        present_info.pResults = nullptr;
        return present_info;
    }

    const uint32_t& VulkanBasicScreenRenderer::getCurrentFrameIndex() const {
        return this->current_frame_index;
    }

    void VulkanBasicScreenRenderer::queueNextFrame() {
        this->current_frame_index = (this->current_frame_index + 1) % this->getMaxQueuedFrame();
    }

    void VulkanBasicScreenRenderer::draw(VulkanGraphicsPipelineBuffer *g_pipeline_buffer) {
        // testing purpose
        if(!g_pipeline_buffer) return;
        std::unordered_map<std::size_t, VulkanGraphicsPipelineObject> &pipelines = g_pipeline_buffer->getAllPipelines();
        if(pipelines.size() == 0) return;

        VkFramebuffer current_swapchain_frame = this->render_window->getFramebuffer(*this->render_window->getAcquiredSwapchain());
        VkRenderPass swapchain_renderpass = this->render_window->getRenderPass();
        VkExtent2D render_area{};
        this->render_window->getDimensions(render_area.width, render_area.height);

        VkRenderPassBeginInfo render_pass_begin{};
        render_pass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin.framebuffer = current_swapchain_frame;

        render_pass_begin.renderPass = swapchain_renderpass;
        render_pass_begin.renderArea.extent = render_area;

        VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
        render_pass_begin.clearValueCount = 1;
        render_pass_begin.pClearValues = &clear_color;
        
        vkCmdBeginRenderPass(this->frame_cmd_buffers[this->current_frame_index], &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);

        VulkanGraphicsPipelineObject& testing_pipeline = pipelines.begin()->second;

        // Binding Pipeline
        vkCmdBindPipeline(this->frame_cmd_buffers[current_frame_index], VK_PIPELINE_BIND_POINT_GRAPHICS, testing_pipeline.vk_pipeline);

        std::vector<VkDeviceSize> offsets = std::vector<VkDeviceSize>(this->vertices_buffer_handles.size(), 0);
        // Binding vertex buffers
        vkCmdBindVertexBuffers(
            this->frame_cmd_buffers[current_frame_index], // command buffer
            0,  // first binding
            this->vertices_buffer_handles.size(), // binding count
            this->vertices_buffer_handles.data(), // binding buffer
            offsets.data()); // binding offset
        
        // Binding index buffer
        vkCmdBindIndexBuffer(
            this->frame_cmd_buffers[current_frame_index], // command buffer
            this->index_buffer_handle,
            0,
            VK_INDEX_TYPE_UINT32);

        // Binding descriptor sets
        vkCmdBindDescriptorSets(
            this->frame_cmd_buffers[current_frame_index], // command buffer
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            testing_pipeline.vk_pipeline_layout,
            0,
            this->uniform_buffer_desc_set.size(),
            this->uniform_buffer_desc_set.data(),
            0,
            nullptr
        );

        vkCmdDrawIndexed(
            this->frame_cmd_buffers[current_frame_index], // command buffer
            static_cast<uint32_t>(this->index_data->size()), 1, 0, 0, 0);
        
        vkCmdEndRenderPass(this->frame_cmd_buffers[current_frame_index]);
    }

    void VulkanBasicScreenRenderer::begin() {
        VkCommandBufferBeginInfo cmd_buffer_begin{};
        cmd_buffer_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmd_buffer_begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        ZERO_VK_CHECK_EXCEPT(vkBeginCommandBuffer(this->frame_cmd_buffers[this->current_frame_index], &cmd_buffer_begin));
    }

    void VulkanBasicScreenRenderer::configureViewportAndScissor(VkExtent2D &extent) {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(extent.width);
        viewport.height = static_cast<float>(extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(this->frame_cmd_buffers[this->current_frame_index], 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = extent;
        vkCmdSetScissor(this->frame_cmd_buffers[this->current_frame_index], 0, 1, &scissor);
    }

    void VulkanBasicScreenRenderer::end() {
        ZERO_VK_CHECK_EXCEPT(vkEndCommandBuffer(this->frame_cmd_buffers[this->current_frame_index]));
    }

    void VulkanBasicScreenRenderer::reset() {
        vkResetCommandBuffer(this->frame_cmd_buffers[this->current_frame_index], 0);
    }

    ZEROResult VulkanBasicScreenRenderer::record(VulkanGraphicsPipelineBuffer *const pipeline_buffer, VulkanCommandRecordingInfo &ret) {
        if(!this->vertices_data || this->vertices_buffer_handles.size() == 0 ||
            !this->index_data || this->index_buffer_handle == VK_NULL_HANDLE) {
            throw std::runtime_error("VulkanBasicScreenRenderer::record() failed, error: vertices data or vertice buffer handle is not mapped");
        }
        this->render_window->tryAcquireSwapchainImage(this->getImageLock(this->current_frame_index));
        this->reset();
        this->begin();
        VkExtent2D render_area{};
        this->render_window->getDimensions(render_area.width, render_area.height);
        this->configureViewportAndScissor(render_area);
        this->draw(pipeline_buffer);
        this->end();
        ret.cmd = { this->frame_cmd_buffers[this->current_frame_index] };
        ret.wait_semaphores = { this->getImageLock(this->current_frame_index) };
        ret.wait_stages = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        ret.signal_semaphores = { this->getRenderingLock(this->current_frame_index) };
        ret.host_signal_fence = { this->getPresentationLock(this->current_frame_index) };
        return { ZERO_SUCCESS, "" };
    }

    void VulkanBasicScreenRenderer::allocateCommandBuffer(const uint32_t &count) {
        if(count == 0) return;
        VkDevice device = this->vulkan_context->getDevice();

        std::size_t emplace_index = this->frame_cmd_buffers.size();
        this->frame_cmd_buffers.resize(emplace_index + count);
        std::vector<VkCommandBuffer> cmd_buffer_emplace(count);

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = this->vk_render_cmd_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        alloc_info.commandBufferCount = count;
        ZERO_VK_CHECK_EXCEPT(vkAllocateCommandBuffers(device, &alloc_info, cmd_buffer_emplace.data()));

        std::size_t ret_index = 0;
        for(std::size_t i = emplace_index; i < this->frame_cmd_buffers.size(); ++i) {
            this->frame_cmd_buffers[i] = cmd_buffer_emplace[ret_index];
            ++ret_index;
        }
    }

    VkFence VulkanBasicScreenRenderer::getPresentationLock(const uint32_t &target_index) {
        return this->vk_presentation_mutex[target_index];
    }
    VkSemaphore VulkanBasicScreenRenderer::getImageLock(const uint32_t &target_index) {
        return this->vk_image_mutex[target_index];
    }
    VkSemaphore VulkanBasicScreenRenderer::getRenderingLock(const uint32_t &target_index) {
        return this->vk_rendering_mutex[target_index];
    }

    void VulkanBasicScreenRenderer::cleanup_syncObjects() {
        VkDevice device = this->vulkan_context->getDevice();

        // concurrency locks count should be synchronized
        uint32_t buffer_count = this->vk_image_mutex.size();
        for(uint32_t i = 0; i < buffer_count; ++i) {
            vkDestroyFence(device, this->vk_presentation_mutex[i], nullptr);
            vkDestroySemaphore(device, this->vk_rendering_mutex[i], nullptr);
            vkDestroySemaphore(device, this->vk_image_mutex[i], nullptr);
        }
    }

    void VulkanBasicScreenRenderer::cleanup() {
        this->cleanup_syncObjects();
        this->render_window->cleanup();
        VulkanRenderer::cleanup();
    }
}