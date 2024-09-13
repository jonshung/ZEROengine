#include "ZEROengine.hpp"
#include "VulkanRenderer_define.hpp"

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
    this->graphical_module.initVulkanGraphicalModule();
    this->graphical_module.getForwardPassPipelineTemplate().createGraphicsPipelinesLayout(this->graphical_module.getVulkanContext().getDevice());

    VulkanGraphicsPipelineBuffer &pipeline_buffer = this->graphical_module.getGraphicsPipelineBuffer();
    VulkanGraphicsPipelineTemplate &pipeline_template = this->graphical_module.getForwardPassPipelineTemplate();

    std::vector<ShaderData> shader_data = { {vert_data, frag_data} };
    std::vector<std::size_t> pipeline_hash = 
    pipeline_buffer.requestGraphicsPipelines(
        graphical_module.getVulkanContext().getDevice(), 
        pipeline_template,
        graphical_module.getVulkanContext().requestSwapChainRenderPass(),
        shader_data);
    this->testing_pipeline = pipeline_buffer.getPipeline(pipeline_hash[0]);
    
    // freeing allocated buffers  
    delete vert_data.first;
    delete frag_data.first;
    // END graphical module
    mainLoop();
}

void ZEROengine::mainLoop() {
    while(!this->quitting_signal) {
        SDL_PollEvent(&this->context_event);
        if(this->context_event.type == SDL_EVENT_QUIT) break;
        this->graphical_module.drawFrame();
    }
}

void ZEROengine::cleanup() {
    this->graphical_module.cleanup();
}

ZEROengine::~ZEROengine() {
    cleanup();
}