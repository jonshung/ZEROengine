#include "VulkanGraphicalModule.hpp"

#include <vulkan/vk_enum_string_helper.h>

void VulkanGraphicalModule::initVulkanGraphicalModule() {
    this->window_context.initWindow(640, 480);
    this->vulkan_context.initVulkan(&this->window_context);
    
    // Rendering contexts
    VulkanRenderContext &render_context = this->vulkan_render_context;
    VulkanRenderContextCreateInfo params{};
    params.device = this->vulkan_context.getDevice();
    params.queue_info = this->vulkan_context.getGraphicalQueue();
    params.submission_queue_count = MAX_QUEUED_FRAME;
    render_context.initVulkanRenderContext(params);

    // Screen Renderer
    VulkanBasicScreenRenderer &screen_renderer = this->vulkan_screen_renderer;

    // Graphics Pipeline buffer
    this->vk_graphics_pipeline_buffer = std::make_unique<VulkanGraphicsPipelineBuffer>();

    // Command pools
    VkCommandPoolCreateInfo cmd_pool_create_info{};
    cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_create_info.queueFamilyIndex = this->vulkan_context.getGraphicalQueue().queueFamilyIndex;
    VkResult rslt = vkCreateCommandPool(this->vulkan_context.getDevice(), &cmd_pool_create_info, nullptr, &this->vk_draw_cmd_pool);
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkCreateCommandPool() failed, err: " + std::string(string_VkResult(rslt)));
    }

    // requesting primary screen recording buffer
    requestCommandBufferAllocation(MAX_QUEUED_FRAME);

    VulkanRendererSettings renderer_settings{}; // empty for now
    screen_renderer.initVulkanRenderer(renderer_settings);
}

void VulkanGraphicalModule::drawFrame() {
    if(this->window_context.isMinimized()) { // drawing on 0-sized framebuffer is dangerous, thus we wait until window is opened again. Also to save computation cost.
        return;
    }
    const uint32_t &current_frame_index = this->vulkan_context.getCurrentFrameIndex();
    VulkanBasicScreenRenderer &screen_renderer = this->vulkan_screen_renderer;
    VulkanRenderContext &render_context = this->vulkan_render_context;

    this->vulkan_context.waitForFence(render_context.getPresentationLock(current_frame_index));
    uint32_t image_index;
    VkResult rslt = this->vulkan_context.acquireSwapChainImageIndex(image_index, render_context.getImageLock(current_frame_index));
    if(rslt == VK_ERROR_OUT_OF_DATE_KHR) {
        handleResize();
        return;
    }
    else if(rslt != VK_SUCCESS && rslt != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("ZEROengine::drawFrame() error, err: " + std::to_string(rslt));
    }
    this->vulkan_context.releaseFence(render_context.getPresentationLock(current_frame_index)); // releasing the lock

    // BEGIN rendering pipeline
    // recording to the secondary command buffer first
    if(current_frame_index >= this->vk_draw_cmd_buffers.size()) return; // cannot draw, something is wrong
    VulkanSecondaryCommandBuffer &screen_cmd_buffer = this->vk_draw_cmd_buffers[current_frame_index];
    VulkanRenderTarget& target = this->vulkan_context.requestSwapChainRenderTargets()[image_index];
    screen_renderer.record(screen_cmd_buffer.buffer, target, this->vk_graphics_pipeline_buffer.get());

    // now record to the render context and submit to hardware
    render_context.reset(current_frame_index);
    render_context.begin(current_frame_index);
    render_context.recordRenderPassCommandBuffer(current_frame_index, screen_cmd_buffer, target);
    render_context.end(current_frame_index);
    render_context.submit(current_frame_index, true);

    this->vulkan_context.presentLatestImage(image_index, render_context.getRenderingLock(current_frame_index));
    this->vulkan_context.queueNextFrame();
    // END rendering pipeline
}

void VulkanGraphicalModule::handleResize() {
    vkDeviceWaitIdle(this->vulkan_context.getDevice());
    this->vulkan_context.VKReload_swapChain();
}

void VulkanGraphicalModule::requestCommandBufferAllocation(const uint32_t &count) {
    if(count == 0) return;
    VkDevice& device = this->vulkan_context.getDevice();

    std::size_t emplace_index = this->vk_draw_cmd_buffers.size();
    this->vk_draw_cmd_buffers.resize(emplace_index + count);
    std::vector<VkCommandBuffer> cmd_buffer_emplace(count);

    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = this->vk_draw_cmd_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    alloc_info.commandBufferCount = count;

    VkResult rslt = vkAllocateCommandBuffers(device, &alloc_info, cmd_buffer_emplace.data());
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkAllocateCommandBuffers() failed, err: " + std::string(string_VkResult(rslt)));
    }

    std::size_t ret_index = 0;
    for(std::size_t i = emplace_index; i < this->vk_draw_cmd_buffers.size(); ++i) {
        this->vk_draw_cmd_buffers[i].buffer = cmd_buffer_emplace[ret_index];
        ++ret_index;
    }
}

void VulkanGraphicalModule::cleanup() {
    vkDeviceWaitIdle(this->vulkan_context.getDevice());

    this->forwardpass_pipeline_template.cleanup(this->getVulkanContext().getDevice());
    (*this->vk_graphics_pipeline_buffer).cleanup(this->vulkan_context.getDevice());

    vkDestroyCommandPool(this->vulkan_context.getDevice(), this->vk_draw_cmd_pool, nullptr);

    this->vulkan_screen_renderer.cleanup(this->vulkan_context.getDevice());

    this->vulkan_render_context.cleanup();
    this->vulkan_context.cleanup();
    this->window_context.cleanup();
    SDL_Quit();
}