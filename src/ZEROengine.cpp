#include "ZEROengine.hpp"
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

void ZEROengine::run() {
    // all of this should be separated into a graphical drawing module - jonshung
    if(SDL_WasInit(SDL_INIT_VIDEO) == 0 && SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL_Init() failed, err: " + std::string(SDL_GetError()));
    }
    this->window_context = std::make_shared<SDLWindowContext>();
    this->window_context->initWindow(640, 480);
    auto frag_data = readFile(std::string(ZEROENGINE_BINARY_TEST_DIR) + "/frag.spv");
    auto vert_data = readFile(std::string(ZEROENGINE_BINARY_TEST_DIR) + "/vert.spv");
    this->graphical_module = std::make_unique<VulkanGraphicalModule>(this->window_context);

    VulkanPipelineBuffer &pipeline_buffer = graphical_module->getScreenRenderer().getGraphicsPipelineBuffer();
    uint32_t layout_index = pipeline_buffer.createGraphicsPipelinesLayout(graphical_module->getVulkanContext().getDevice());
    std::vector<size_t> pipeline_indices = pipeline_buffer.createGraphicsPipelines(
            graphical_module->getVulkanContext().getDevice(), 
            pipeline_buffer.getPipelineLayout(layout_index),
            graphical_module->getVulkanContext().requestSwapChainRenderPass(),
            { {vert_data, frag_data} });
    this->testing_pipeline = pipeline_buffer.getPipeline(pipeline_indices[0]);
    
    // freeing allocated buffers
    delete std::get<0>(frag_data);
    delete std::get<0>(vert_data);
    mainLoop();
}

void ZEROengine::mainLoop() {
    while(!this->quitting_signal) {
        SDL_PollEvent(&this->context_event);
        if(this->context_event.type == SDL_EVENT_QUIT) break;
        this->graphical_module->drawFrame();
    }
}

void ZEROengine::cleanup() {
    this->graphical_module->cleanup();
}

ZEROengine::~ZEROengine() {
    cleanup();
}