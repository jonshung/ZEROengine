#include "VulkanBasicScreenRenderer.hpp"

#include <unordered_map>

#include <vulkan/vk_enum_string_helper.h>

void VulkanBasicScreenRenderer::initVulkanRenderer(const VulkanRendererSettings &settings, VulkanContext &vulkan_context, const uint32_t &queue_family_index) {
    VulkanRenderer::initVulkanRenderer(settings, vulkan_context, queue_family_index);
    this->vk_swapchain_render_pass = std::make_shared<VkRenderPass>();
    initSwapChain();
    initSwapChainRenderPass();
    initSwapChainRenderTargets();
    createConcurrencyLock(this->getMaxQueuedFrame()); // locks for each frame
    allocateSecondaryCommandBuffer(this->getMaxQueuedFrame());
}

void VulkanBasicScreenRenderer::initSwapChain() {
    VulkanContext::SwapChainSupportDetails swap_chain_support = this->vulkan_context->querySwapChainSupport(this->vulkan_context->getPhysicalDevice());

    VkSurfaceFormatKHR surface_format = selectSwapChainSurfaceFormat(swap_chain_support.formats);
    VkPresentModeKHR present_mode = selectSwapChainPresentationMode(swap_chain_support.present_modes);
    VkExtent2D extent = this->vk_swapchain_extent = selectSwapChainExtent(swap_chain_support.capabilities);
    this->vk_swapchain_image_format = surface_format.format;
    
    uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
    if(swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount) 
        image_count = swap_chain_support.capabilities.maxImageCount;
    VkSwapchainCreateInfoKHR swap_chain_create_info{};
    swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swap_chain_create_info.surface = this->vulkan_context->getWindowSurface();
    swap_chain_create_info.minImageCount = image_count;
    swap_chain_create_info.imageFormat = surface_format.format;
    swap_chain_create_info.imageColorSpace = surface_format.colorSpace;
    swap_chain_create_info.imageExtent = extent;
    swap_chain_create_info.imageArrayLayers = 1;
    swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    if (this->vulkan_context->getGraphicalQueue().queueFamilyIndex != this->vulkan_context->getPresentationQueue().queueFamilyIndex) {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_chain_create_info.queueFamilyIndexCount = 2;
        uint32_t queue_indices[2] = {this->vulkan_context->getGraphicalQueue().queueFamilyIndex, this->vulkan_context->getPresentationQueue().queueFamilyIndex};
        swap_chain_create_info.pQueueFamilyIndices = queue_indices;
    } else {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swap_chain_create_info.queueFamilyIndexCount = 0; // Optional
        swap_chain_create_info.pQueueFamilyIndices = nullptr; // Optional
    }
    swap_chain_create_info.preTransform = swap_chain_support.capabilities.currentTransform;
    swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swap_chain_create_info.presentMode = present_mode;
    swap_chain_create_info.clipped = VK_TRUE;
    swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;
    VkResult rslt;
    if((rslt = vkCreateSwapchainKHR(this->vulkan_context->getDevice(), &swap_chain_create_info, nullptr, &this->vk_swapchain)) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateSwapchainKHR() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanBasicScreenRenderer::initSwapChainRenderPass() {
    std::array<VkAttachmentDescription, 2> attachments = {};

    attachments[0].format = this->vk_swapchain_image_format;
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
    render_pass_info.attachmentCount = static_cast<uint32_t>(this->enable_depth_stencil_subpass ? 2 : 1);
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = static_cast<uint32_t>(this->enable_depth_stencil_subpass ? 2 : 1);
    render_pass_info.pDependencies = subpass_dep.data();
    VkResult rslt = vkCreateRenderPass(this->vulkan_context->getDevice(), &render_pass_info, nullptr, this->vk_swapchain_render_pass.get());
    if (rslt != VK_SUCCESS) {
        throw std::runtime_error("vkCreateRenderPass failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanBasicScreenRenderer::initSwapChainRenderTargets() {
    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(this->vulkan_context->getDevice(), this->vk_swapchain, &image_count, nullptr);
    this->vk_swapchain_render_targets.resize(image_count);
    std::vector<VkImage> image_buffer(image_count);
    vkGetSwapchainImagesKHR(this->vulkan_context->getDevice(), this->vk_swapchain, &image_count, image_buffer.data());

    VkResult rslt;
    for(std::size_t i = 0; i < image_buffer.size(); ++i) {
        this->vk_swapchain_render_targets[i].image = image_buffer[i];
        this->vk_swapchain_render_targets[i].framebuffer_extent = this->vk_swapchain_extent;
        this->vk_swapchain_render_targets[i].framebuffer_format = this->vk_swapchain_image_format;
        this->vk_swapchain_render_targets[i].render_pass = this->vk_swapchain_render_pass;

        VkImageViewCreateInfo img_view_create_info{};
        img_view_create_info.format = this->vk_swapchain_image_format;
        img_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        img_view_create_info.image = image_buffer[i];
        img_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        img_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        img_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        img_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        img_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        img_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        img_view_create_info.subresourceRange.layerCount = 1;
        img_view_create_info.subresourceRange.levelCount = 1;
        img_view_create_info.subresourceRange.baseMipLevel = 0;
        img_view_create_info.subresourceRange.baseArrayLayer = 0;
        if((rslt = vkCreateImageView(this->vulkan_context->getDevice(), &img_view_create_info, nullptr, &this->vk_swapchain_render_targets[i].image_view)) != VK_SUCCESS) {
            throw std::runtime_error("vkCreateImageView() failed, err: " + std::string(string_VkResult(rslt)));
        }

        VkImageView* img_view_ref = &this->vk_swapchain_render_targets[i].image_view;
        VkFramebufferCreateInfo framebuffers_create_info{};
        framebuffers_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffers_create_info.width = this->vk_swapchain_extent.width;
        framebuffers_create_info.height = this->vk_swapchain_extent.height;
        framebuffers_create_info.layers = 1;
        framebuffers_create_info.attachmentCount = 1;
        framebuffers_create_info.pAttachments = img_view_ref;
        framebuffers_create_info.renderPass = *this->vk_swapchain_render_pass;
        if((rslt = vkCreateFramebuffer(
            this->vulkan_context->getDevice(), 
            &framebuffers_create_info, 
            nullptr, 
            &this->vk_swapchain_render_targets[i].framebuffer))
         != VK_SUCCESS) {
            throw std::runtime_error("vkCreateFramebuffer() failed, err: + " + std::string(string_VkResult(rslt)));
        }
    }
}

VkSurfaceFormatKHR VulkanBasicScreenRenderer::selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) {
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    return formats[0];
}

VkPresentModeKHR VulkanBasicScreenRenderer::selectSwapChainPresentationMode(const std::vector<VkPresentModeKHR> &modes) {
    for(const auto& mode : modes) {
        if(mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanBasicScreenRenderer::selectSwapChainExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    int w, h;
    this->vulkan_context->getWindowContext()->getFramebufferSize(w, h);
    VkExtent2D actual_extent = {
            static_cast<uint32_t>(w),
            static_cast<uint32_t>(h)
    };
    actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actual_extent;
}

VkResult VulkanBasicScreenRenderer::acquireSwapChainImageIndex(uint32_t &index, VkSemaphore semaphore_lock) {
    return vkAcquireNextImageKHR(this->vulkan_context->getDevice(), this->vk_swapchain, UINT64_MAX, semaphore_lock, VK_NULL_HANDLE, &index);
}

void VulkanBasicScreenRenderer::presentImage(VkSemaphore semaphore_lock) {
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &semaphore_lock;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &this->vk_swapchain;
    present_info.pImageIndices = &this->swapchain_acquired_image;
    present_info.pResults = nullptr;
    VkResult rslt = vkQueuePresentKHR(this->vulkan_context->getPresentationQueue().queue, &present_info);
    (void)(rslt); // debug
}

const uint32_t& VulkanBasicScreenRenderer::getCurrentFrameIndex() const {
    return this->current_frame_index;
}

void VulkanBasicScreenRenderer::queueNextFrame() {
    this->current_frame_index = (this->current_frame_index + 1) % this->getMaxQueuedFrame();
}

void VulkanBasicScreenRenderer::reload_swapChain() {
    // should only be called in context when none of these is in used.
    cleanup_swapChain();

    initSwapChain();
    initSwapChainRenderTargets();
}

void VulkanBasicScreenRenderer::cleanup_swapChain() {
    for(auto &render_target : this->vk_swapchain_render_targets) {
        vkDestroyFramebuffer(this->vulkan_context->getDevice(), render_target.framebuffer, nullptr);
        vkDestroyImageView(this->vulkan_context->getDevice(), render_target.image_view, nullptr);
    }
    vkDestroySwapchainKHR(this->vulkan_context->getDevice(), this->vk_swapchain, nullptr);
}

void VulkanBasicScreenRenderer::draw(VulkanGraphicsPipelineBuffer *g_pipeline_buffer) {
    // testing purpose
    if(!g_pipeline_buffer) return;
    std::unordered_map<std::size_t, VkPipeline> &pipelines = g_pipeline_buffer->getAllPipelines();
    if(pipelines.size() == 0) return;
    VkPipeline& testing_pipeline = pipelines.begin()->second;
    vkCmdBindPipeline(this->frame_cmd_buffers[current_frame_index].buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, testing_pipeline);
    vkCmdDraw(this->frame_cmd_buffers[current_frame_index].buffer, 3, 1, 0 ,0);
}

void VulkanBasicScreenRenderer::begin() {
    VkCommandBufferBeginInfo cmd_buffer_begin{};
    cmd_buffer_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buffer_begin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    VkCommandBufferInheritanceInfo inheritance_info{};
    inheritance_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inheritance_info.framebuffer = this->vk_swapchain_render_targets[this->swapchain_acquired_image].framebuffer;
    inheritance_info.renderPass = (*this->vk_swapchain_render_targets[this->swapchain_acquired_image].render_pass); // we specify our own render pass for each renderer
    inheritance_info.subpass = 0;
    cmd_buffer_begin.pInheritanceInfo = &inheritance_info;

    VkResult rslt = vkBeginCommandBuffer(this->frame_cmd_buffers[this->current_frame_index].buffer, &cmd_buffer_begin);
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
    vkCmdSetViewport(this->frame_cmd_buffers[this->current_frame_index].buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = extent;
    vkCmdSetScissor(this->frame_cmd_buffers[this->current_frame_index].buffer, 0, 1, &scissor);
}

void VulkanBasicScreenRenderer::end() {
    VkResult rslt;
    if ((rslt = vkEndCommandBuffer(this->frame_cmd_buffers[this->current_frame_index].buffer)) != VK_SUCCESS) {
        throw std::runtime_error("vkEndCommandBuffer() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanBasicScreenRenderer::reset() {
    vkResetCommandBuffer(this->frame_cmd_buffers[this->current_frame_index].buffer, 0);
}

std::vector<std::pair<VulkanRenderTarget*, VulkanSecondaryCommandBuffer>> VulkanBasicScreenRenderer::record(VulkanGraphicsPipelineBuffer *const pipeline_buffer) {
    this->reset();
    this->begin();
    this->configureViewportAndScissor(this->vk_swapchain_render_targets[this->swapchain_acquired_image].framebuffer_extent);
    this->draw(pipeline_buffer);
    this->end();
    return { { &this->vk_swapchain_render_targets[this->swapchain_acquired_image], this->frame_cmd_buffers[this->current_frame_index] } };
}

void VulkanBasicScreenRenderer::createConcurrencyLock(const uint32_t &count) {
    VkDevice& device = this->vulkan_context->getDevice();

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

void VulkanBasicScreenRenderer::allocateSecondaryCommandBuffer(const uint32_t &count) {
    if(count == 0) return;
    VkDevice& device = this->vulkan_context->getDevice();

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
        this->frame_cmd_buffers[i].buffer = cmd_buffer_emplace[ret_index];
        ++ret_index;
    }
}

void VulkanBasicScreenRenderer::cleanup_concurrency_locks() {
    VkDevice& device = this->vulkan_context->getDevice();

    // concurrency locks count should be synchronized
    uint32_t buffer_count = this->vk_image_mutex.size();
    for(uint32_t i = 0; i < buffer_count; ++i) {
        vkDestroyFence(device, this->vk_presentation_mutex[i], nullptr);
        vkDestroySemaphore(device, this->vk_rendering_mutex[i], nullptr);
        vkDestroySemaphore(device, this->vk_image_mutex[i], nullptr);
    }
}

void VulkanBasicScreenRenderer::handleResize() {
    vkDeviceWaitIdle(this->vulkan_context->getDevice());
    this->reload_swapChain();
}

void VulkanBasicScreenRenderer::cleanup() {
    this->cleanup_concurrency_locks();
    vkDestroyRenderPass(this->vulkan_context->getDevice(), *this->vk_swapchain_render_pass, nullptr);
    this->cleanup_swapChain();
    VulkanRenderer::cleanup();
}