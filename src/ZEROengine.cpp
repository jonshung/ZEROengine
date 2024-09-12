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
    // BEGIN graphical module
    if(SDL_WasInit(SDL_INIT_VIDEO) == 0 && SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL_Init() failed, err: " + std::string(SDL_GetError()));
    }
    auto frag_data = readFile(std::string(ZEROENGINE_BINARY_TEST_DIR) + "/frag.spv");
    auto vert_data = readFile(std::string(ZEROENGINE_BINARY_TEST_DIR) + "/vert.spv");
    this->graphical_module = std::make_unique<VulkanGraphicalModule>();
    this->graphical_module->initVulkanGraphicalModule();

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
    // END graphical module
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