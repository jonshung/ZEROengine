#include "VulkanGraphicalModule.hpp"

VulkanGraphicalModule::VulkanGraphicalModule(const std::shared_ptr<WindowContext> window_ctx) {
    this->window_context = window_ctx;
    this->vulkan_context.initVulkan(this->window_context.get());

    VulkanBasicScreenRenderer &screen_renderer = this->vulkan_screen_renderer;

    VulkanRenderContext &render_context = this->vulkan_render_context;
    VulkanRenderContextCreateInfo params{};
    params.device = this->vulkan_context.getDevice();
    params.queue_info = this->vulkan_context.getGraphicalQueue();
    params.submission_queue_count = MAX_QUEUED_FRAME;
    render_context.initVulkanRenderContext(params);
    this->frame_command_buffer_indices = render_context.requestCommandBufferAllocation(MAX_QUEUED_FRAME);

    VulkanRendererSettings renderer_settings{}; // empty for now
    screen_renderer.initVulkanRenderer(renderer_settings);
}

void VulkanGraphicalModule::drawFrame() {
    if(this->window_context->isMinimized()) { // drawing on 0-sized framebuffer is dangerous, thus we wait until window is opened again. Also to save computation cost.
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
    VulkanSecondaryCommandBuffer* screen_cmd_buffer;
    render_context.getCommandBuffer(this->frame_command_buffer_indices[current_frame_index], &screen_cmd_buffer);

    if(!screen_cmd_buffer) return; // NULL handle here
    VulkanRenderTarget& target = this->vulkan_context.requestSwapChainRenderTargets()[image_index];
    screen_renderer.record(screen_cmd_buffer->buffer, target);
    screen_cmd_buffer->ready = true;

    // now record to the render context and submit to hardware
    render_context.reset(current_frame_index);
    render_context.begin(current_frame_index);
    render_context.recordRenderPassCommandBuffer(current_frame_index, target);
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

void VulkanGraphicalModule::cleanup() {
    vkDeviceWaitIdle(this->vulkan_context.getDevice());
    this->vulkan_screen_renderer.cleanup(this->vulkan_context.getDevice());

    this->vulkan_render_context.cleanup();
    this->vulkan_context.cleanup();
    this->window_context->cleanup();
    SDL_Quit();
}