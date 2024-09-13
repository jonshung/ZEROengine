#include "VulkanGraphicalModule.hpp"

#include <vulkan/vk_enum_string_helper.h>

void VulkanGraphicalModule::initVulkanGraphicalModule() {
    this->window_context.initWindow(640, 480);
    this->vulkan_context.initVulkan(&this->window_context);

    // Screen Renderer
    VulkanBasicScreenRenderer &screen_renderer = this->vulkan_screen_renderer;
    VulkanRendererSettings renderer_settings{false};
    screen_renderer.initVulkanRenderer(renderer_settings, this->vulkan_context, this->getVulkanContext().getGraphicalQueue().queueFamilyIndex);
    
    // Rendering contexts
    VulkanRenderContext &render_context = this->vulkan_render_context;
    VulkanRenderContextCreateInfo params{};
    params.device = this->vulkan_context.getDevice();
    params.queue_info = this->vulkan_context.getGraphicalQueue();
    params.primary_buffer_count = screen_renderer.getMaxQueuedFrame();
    render_context.initVulkanRenderContext(params);

    // Graphics Pipeline buffer
    this->vk_graphics_pipeline_buffer = std::make_unique<VulkanGraphicsPipelineBuffer>();
}

void VulkanGraphicalModule::drawFrame() {
    if(this->window_context.isMinimized()) { // drawing on 0-sized framebuffer is dangerous, thus we wait until window is opened again. Also to save computation cost.
        return;
    }
    const uint32_t &current_frame_index = this->vulkan_screen_renderer.getCurrentFrameIndex();
    VulkanBasicScreenRenderer &screen_renderer = this->vulkan_screen_renderer;
    VulkanRenderContext &render_context = this->vulkan_render_context;

    this->vulkan_context.waitForFence(screen_renderer.getPresentationLock(current_frame_index));
    VkSemaphore image_lock = screen_renderer.getImageLock(current_frame_index);
    VkResult rslt = screen_renderer.queryAcquireAndStoreFrame(image_lock); // trying to acquire and internally store a render target in the renderer.
    if(rslt == VK_ERROR_OUT_OF_DATE_KHR) {
        handleResize();
        return;
    }

    else if(rslt != VK_SUCCESS && rslt != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("ZEROengine::drawFrame() error, err: " + std::to_string(rslt));
    }
    this->vulkan_context.releaseFence(screen_renderer.getPresentationLock(current_frame_index)); // releasing the lock

    // BEGIN rendering pipeline
    // recording to the secondary command buffer first
    std::vector<std::pair<VulkanRenderTarget*, VulkanSecondaryCommandBuffer>> 
    draw_cmd_buffers = screen_renderer.record(this->vk_graphics_pipeline_buffer.get());

    // now record to the render context and submit to hardware
    render_context.reset(current_frame_index);
    render_context.begin(current_frame_index);
    for(auto &[ pRenderTarget, cmd_buffer ] : draw_cmd_buffers) {
        if(!pRenderTarget) continue;
        render_context.recordRenderPassCommandBuffer(current_frame_index, cmd_buffer, *pRenderTarget);
    }
    render_context.end(current_frame_index);
    render_context.submit(
        current_frame_index, 
        screen_renderer.getImageLock(current_frame_index), 
        screen_renderer.getRenderingLock(current_frame_index),
        screen_renderer.getPresentationLock(current_frame_index)
    );

    screen_renderer.presentImage(screen_renderer.getRenderingLock(current_frame_index));
    screen_renderer.queueNextFrame();
    // END rendering pipeline
}

void VulkanGraphicalModule::handleResize() {
    this->getScreenRenderer().handleResize();
}

void VulkanGraphicalModule::cleanup() {
    vkDeviceWaitIdle(this->vulkan_context.getDevice());

    this->forwardpass_pipeline_template.cleanup(this->getVulkanContext().getDevice());
    (*this->vk_graphics_pipeline_buffer).cleanup(this->vulkan_context.getDevice());

    this->vulkan_screen_renderer.cleanup();

    this->vulkan_render_context.cleanup();
    this->vulkan_context.cleanup();
    this->window_context.cleanup();
    SDL_Quit();
}