#include "BasicApp.hpp"
#include "VulkanRenderer_def.hpp"

#include "glm/glm.hpp"

#include <vector>
#include <array>
#include <iostream>

const std::vector<BaseVertex> triangle = {
    { {0.0, -0.5f}, {1.0f, 0.0f, 0.0f} },
    { {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f} },
    { {-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} }
};

void BasicApp::run() {
    // all of this should be separated into a graphical drawing module - jonshung

    if(SDL_WasInit(SDL_INIT_VIDEO) == 0 && SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL_Init() failed, err: " + std::string(SDL_GetError()));
    }
    this->window_context.initWindow(640, 480);
    this->vulkan_context.initVulkan(&this->window_context);
    auto frag_data = readFile(std::string(PROJECT_BINARY_TEST_DIR) + "/frag.spv");
    auto vert_data = readFile(std::string(PROJECT_BINARY_TEST_DIR) + "/vert.spv");

    VulkanRenderer& screen_renderer = this->vulkan_screen_renderer;
    VkQueueInfo& graphic_queue_info = this->vulkan_context.getGraphicalQueue();

    VulkanRendererDependencies dependencies{};
    dependencies.image_views = this->vulkan_context.requestSwapChainImageViews();
    dependencies.extent = this->vulkan_context.requestSwapChainExtent();
    dependencies.framebuffer_format = this->vulkan_context.requestSwapChainImageFormat();

    VulkanRendererCreateInfo info{};
    info.device = this->vulkan_context.getDevice();
    info.queue_info = graphic_queue_info;
    info.dependencies = dependencies;
    screen_renderer.initVulkanRenderer(&info);

    this->vk_graphics_pipeline_buffer = std::make_unique<VulkanPipelineBuffer>(this->vulkan_screen_renderer.getRenderPass());

    uint32_t layout_index = (*this->vk_graphics_pipeline_buffer).createGraphicsPipelinesLayout(vulkan_context.getDevice());
    std::vector<size_t> pipeline_indices = (*this->vk_graphics_pipeline_buffer).createGraphicsPipelines(
            vulkan_context.getDevice(), 
            (*this->vk_graphics_pipeline_buffer).getPipelineLayout(layout_index),
            screen_renderer.getRenderPass(),
            { {vert_data, frag_data} });
    this->testing_pipeline = (*this->vk_graphics_pipeline_buffer).getPipeline(pipeline_indices[0]);
    // freeing allocated buffers
    delete std::get<0>(frag_data);
    delete std::get<0>(vert_data);
    mainLoop();
}

void BasicApp::mainLoop() {
    while(!this->quitting_signal) {
        SDL_PollEvent(&this->context_event);
        if(this->context_event.type == SDL_EVENT_QUIT) break;
        drawFrame();
    }
}

void BasicApp::drawFrame() {
    if(this->window_context.isMinimized()) { // drawing on 0-sized framebuffer is dangerous, thus we wait until window is opened again. Also to save computation cost.
        return;
    }
    const uint32_t &current_frame_index = this->vulkan_context.getCurrentFrameIndex();
    VulkanRenderer& screen_renderer = this->vulkan_screen_renderer;
    this->vulkan_context.waitForFence(screen_renderer.getPresentationLock(current_frame_index));
    uint32_t image_index;
    VkResult rslt = this->vulkan_context.acquireSwapChainImageIndex(image_index, screen_renderer.getImageLock(current_frame_index));
    if(rslt == VK_ERROR_OUT_OF_DATE_KHR) {
        handleResize();
        return;
    }
    else if(rslt != VK_SUCCESS && rslt != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("BasicApp::drawFrame() error, err: " + std::to_string(rslt));
    }
    this->vulkan_context.releaseFence(screen_renderer.getPresentationLock(current_frame_index)); // releasing the lock

    // BEGIN rendering pipeline
    screen_renderer.resetRenderCommandBuffer(current_frame_index);
    screen_renderer.beginRenderPassCommandBuffer(current_frame_index, image_index);
    screen_renderer.recordPipelineRenderCommandBuffer(current_frame_index, this->testing_pipeline);
    screen_renderer.endRenderPassCommandBuffer(current_frame_index);
    
    screen_renderer.submitRenderCommandBuffer(current_frame_index);
    this->vulkan_context.presentLatestImage(image_index, screen_renderer.getRenderingLock(current_frame_index));
    this->vulkan_context.queueNextFrame();
    // END rendering pipeline
}

void BasicApp::handleResize() {
    vkDeviceWaitIdle(this->vulkan_context.getDevice());
    // invalidating framebuffer on screen renderer
    this->vulkan_screen_renderer.invalidateFramebuffer();
    this->vulkan_screen_renderer.cleanup_framebuffers();

    this->vulkan_context.VKReload_swapChain();

    // reloading dependencies and framebuffers creation on renderer
    VulkanRendererDependencies dependencies{};
    dependencies.image_views = this->vulkan_context.requestSwapChainImageViews();
    dependencies.framebuffer_format = this->vulkan_context.requestSwapChainImageFormat();
    dependencies.extent = this->vulkan_context.requestSwapChainExtent();

    this->vulkan_screen_renderer.reloadDependencies(dependencies);
    this->vulkan_screen_renderer.reloadFramebuffers();
    this->vulkan_screen_renderer.validateFramebuffer();
}

void BasicApp::cleanup() {
    vkDeviceWaitIdle(this->vulkan_context.getDevice());
    (*this->vk_graphics_pipeline_buffer).cleanup(this->vulkan_context.getDevice());
    this->vulkan_screen_renderer.cleanup();

    this->vulkan_context.cleanup();
    this->window_context.cleanup();
    SDL_Quit();
}

BasicApp::~BasicApp() {
    cleanup();
}