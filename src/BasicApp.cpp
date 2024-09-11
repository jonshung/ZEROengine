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

    VulkanBasicScreenRenderer &screen_renderer = this->vulkan_screen_renderer;
    VulkanPipelineBuffer &pipeline_buffer = screen_renderer.getGraphicsPipelineBuffer();

    VulkanRenderContext &render_context = this->vulkan_render_context;
    VulkanRenderContextCreateInfo params{};
    params.device = this->vulkan_context.getDevice();
    params.queue_info = this->vulkan_context.getGraphicalQueue();
    params.submission_queue_count = MAX_QUEUED_FRAME;
    render_context.initVulkanRenderContext(params);
    this->frame_command_buffer_indices = render_context.requestCommandBufferAllocation(MAX_QUEUED_FRAME);

    VulkanRendererSettings renderer_settings{}; // empty for now
    screen_renderer.initVulkanRenderer(renderer_settings);

    uint32_t layout_index = pipeline_buffer.createGraphicsPipelinesLayout(vulkan_context.getDevice());
    std::vector<size_t> pipeline_indices = pipeline_buffer.createGraphicsPipelines(
            vulkan_context.getDevice(), 
            pipeline_buffer.getPipelineLayout(layout_index),
            vulkan_context.requestSwapChainRenderPass(),
            { {vert_data, frag_data} });
    this->testing_pipeline = pipeline_buffer.getPipeline(pipeline_indices[0]);
    
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
        throw std::runtime_error("BasicApp::drawFrame() error, err: " + std::to_string(rslt));
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

void BasicApp::handleResize() {
    vkDeviceWaitIdle(this->vulkan_context.getDevice());
    this->vulkan_context.VKReload_swapChain();
}

void BasicApp::cleanup() {
    vkDeviceWaitIdle(this->vulkan_context.getDevice());
    this->vulkan_screen_renderer.cleanup(this->vulkan_context.getDevice());

    this->vulkan_render_context.cleanup();
    this->vulkan_context.cleanup();
    this->window_context.cleanup();
    SDL_Quit();
}

BasicApp::~BasicApp() {
    cleanup();
}