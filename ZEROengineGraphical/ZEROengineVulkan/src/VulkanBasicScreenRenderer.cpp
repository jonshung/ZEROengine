#include "zeroengine_vulkan/VulkanBasicScreenRenderer.hpp"

#include <unordered_map>

#include <vulkan/vk_enum_string_helper.h>

namespace ZEROengine {
    VulkanBasicScreenRenderer::VulkanBasicScreenRenderer() :
    VulkanRenderer(),
    frame_cmd_buffers{},
    render_window{},
    current_frame_index{0},
    acquired_swapchain_index{},
    vertices_data{nullptr},
    vertices_buffer_handles{},
    index_data{nullptr},
    index_buffer_handle{},
    uniform_buffer_desc_set{},
    vk_image_mutex{},
    vk_rendering_mutex{},
    vk_presentation_mutex{}
    {}

    ZEROResult VulkanBasicScreenRenderer::initVulkanRenderer(VulkanContext *vulkan_context) {
        VulkanRenderer::initVulkanRenderer(vulkan_context);
        allocateSecondaryCommandBuffer(this->getMaxQueuedFrame());
        return { ZERO_SUCCESS, "" };
    }

    void VulkanBasicScreenRenderer::createSyncObjects(const uint32_t &count) {
        VkDevice device{};
        this->vulkan_context->getDevice(device);

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

    VkPresentInfoKHR VulkanBasicScreenRenderer::getPresentImageInfo() {
        VkSwapchainKHR swapchain{};
        this->render_window.getSwapchain(swapchain);

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &this->vk_rendering_mutex[this->current_frame_index];
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain;
        present_info.pImageIndices = &this->acquired_swapchain_index;
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
    }

    void VulkanBasicScreenRenderer::begin() {
        VkFramebuffer current_swapchain_frame;{}
        this->render_window.getFramebuffer(this->acquired_swapchain_index, current_swapchain_frame);
        VkRenderPass swapchain_renderpass{};
        this->render_window.getRenderPass(swapchain_renderpass);

        VkCommandBufferBeginInfo cmd_buffer_begin{};
        cmd_buffer_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmd_buffer_begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

        VkCommandBufferInheritanceInfo inheritance_info{};
        inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        inheritance_info.framebuffer = current_swapchain_frame;
        inheritance_info.renderPass = swapchain_renderpass;
        inheritance_info.subpass = 0;
        cmd_buffer_begin.pInheritanceInfo = &inheritance_info;

        VkResult rslt = vkBeginCommandBuffer(this->frame_cmd_buffers[this->current_frame_index], &cmd_buffer_begin);
        if(rslt != VK_SUCCESS) {
            throw std::runtime_error("vkBeginCommandBuffer() failed, err: " + std::string(string_VkResult(rslt)));
        }
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
        VkResult rslt{};
        if ((rslt = vkEndCommandBuffer(this->frame_cmd_buffers[this->current_frame_index])) != VK_SUCCESS) {
            throw std::runtime_error("vkEndCommandBuffer() failed, err: " + std::string(string_VkResult(rslt)));
        }
    }

    void VulkanBasicScreenRenderer::reset() {
        vkResetCommandBuffer(this->frame_cmd_buffers[this->current_frame_index], 0);
    }

    ZEROResult VulkanBasicScreenRenderer::record(VulkanGraphicsPipelineBuffer *const pipeline_buffer, std::vector<VulkanGraphicsRecordingInfo> &ret) {
        if(!this->vertices_data || this->vertices_buffer_handles.size() == 0 ||
            !this->index_data || this->index_buffer_handle == VK_NULL_HANDLE) {
            throw std::runtime_error("VulkanBasicScreenRenderer::record() failed, error: vertices data or vertice buffer handle is not mapped");
        }
        this->reset();
        this->begin();
        VkExtent2D render_area{};
        this->render_window.getDimensions(render_area.width, render_area.height);
        this->configureViewportAndScissor(render_area);
        this->draw(pipeline_buffer);
        this->end();
        VkRenderPass renderpass{};
        this->render_window.getRenderPass(renderpass);
        VkFramebuffer framebuffer{};
        this->render_window.getFramebuffer(this->acquired_swapchain_index, framebuffer);
        ret.push_back({ this->frame_cmd_buffers[this->current_frame_index], renderpass, render_area, framebuffer });
        return { ZERO_SUCCESS, "" };
    }

    void VulkanBasicScreenRenderer::allocateSecondaryCommandBuffer(const uint32_t &count) {
        if(count == 0) return;
        VkDevice device{};
        this->vulkan_context->getDevice(device);

        std::size_t emplace_index = this->frame_cmd_buffers.size();
        this->frame_cmd_buffers.resize(emplace_index + count);
        std::vector<VkCommandBuffer> cmd_buffer_emplace(count);

        VkCommandBufferAllocateInfo alloc_info{};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandPool = this->vk_render_cmd_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        alloc_info.commandBufferCount = count;

        VkResult rslt = vkAllocateCommandBuffers(device, &alloc_info, cmd_buffer_emplace.data());
        if(rslt != VK_SUCCESS) {
            throw std::runtime_error("vkAllocateCommandBuffers() failed, err: " + std::string(string_VkResult(rslt)));
        }

        std::size_t ret_index = 0;
        for(std::size_t i = emplace_index; i < this->frame_cmd_buffers.size(); ++i) {
            this->frame_cmd_buffers[i] = cmd_buffer_emplace[ret_index];
            ++ret_index;
        }
    }

    void VulkanBasicScreenRenderer::cleanup_syncObjects() {
        VkDevice device{};
        this->vulkan_context->getDevice(device);

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
        VulkanRenderer::cleanup();
    }
}