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
    if(SDL_WasInit(SDL_INIT_VIDEO) == 0 && SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL_Init() failed, err: " + std::string(SDL_GetError()));
    }
    window_context.initWindow(640, 480);
    vulkan_context.initVulkan(&window_context);
    auto frag_data = readFile(std::string(PROJECT_BINARY_TEST_DIR) + "/frag.spv");
    auto vert_data = readFile(std::string(PROJECT_BINARY_TEST_DIR) + "/vert.spv");

    VulkanPipelineBuffer& graphics_pipeline_buffer = vulkan_context.getGraphicsPipelineBuffer();
    VulkanRenderer& screen_renderer = this->vulkan_context.getScreenRenderer();
    uint32_t layout_index = graphics_pipeline_buffer.createGraphicsPipelinesLayout(vulkan_context.getDevice());
    std::vector<size_t> pipeline_indices = graphics_pipeline_buffer.createGraphicsPipelines(
            vulkan_context.getDevice(), 
            graphics_pipeline_buffer.getPipelineLayout(layout_index),
            screen_renderer.getRenderPass(),
            { {vert_data, frag_data} });
    this->testing_pipeline = graphics_pipeline_buffer.getPipeline(pipeline_indices[0]);
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
    VulkanRenderer& screen_renderer = this->vulkan_context.getScreenRenderer();
    this->vulkan_context.waitForFence(screen_renderer.getPresentationLock(current_frame_index));
    uint32_t image_index;
    VkResult rslt = this->vulkan_context.acquireSwapChainImageIndex(image_index, screen_renderer.getImageLock(current_frame_index));
    if(rslt == VK_ERROR_OUT_OF_DATE_KHR) {
        this->vulkan_context.VKReload_swapChain();
        return;
    }
    else if(rslt != VK_SUCCESS && rslt != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("BasicApp::drawFrame() error, err: " + std::to_string(rslt));
    }
    this->vulkan_context.releaseFence(screen_renderer.getPresentationLock(current_frame_index)); // releasing the lock

    screen_renderer.resetRenderCommandBuffer(current_frame_index);
    screen_renderer.beginRenderPassCommandBuffer(current_frame_index, image_index);
    screen_renderer.recordPipelineRenderCommandBuffer(current_frame_index, this->testing_pipeline);
    screen_renderer.endRenderPassCommandBuffer(current_frame_index);
    
    screen_renderer.submitRenderCommandBuffer(current_frame_index);
    this->vulkan_context.presentLatestImage(image_index, screen_renderer.getRenderingLock(current_frame_index));
    this->vulkan_context.queueNextFrame();
}

void BasicApp::cleanup() {
    this->vulkan_context.cleanup();
    this->window_context.cleanup();
    SDL_Quit();
}

BasicApp::~BasicApp() {
    cleanup();
}