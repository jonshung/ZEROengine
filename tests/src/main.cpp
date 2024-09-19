#include "zeroengine_core/ZEROengine.hpp"
#include "zeroengine_sdl/SDL_VulkanWindowContext.hpp"
#include "zeroengine_vulkan/VulkanGraphicalModule.hpp"

#include <iostream>
#include <string>

/**
 * @brief A simple utitlity to load GPU program in the form of SPIR-V.
 * 
 * @param filename Source file path.
 * @return std::pair<char*, std::size_t>. A pointer to the source blob and its size.
 */
static std::pair<char*, std::size_t> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }
    std::size_t f_size = (std::size_t) file.tellg();
    char* buffer = new char[f_size]; // DANGEROUS, i.e. please handle in the future - jonshung
    if(buffer == nullptr) {
        throw std::runtime_error("readFile() failed, err: cannot allocate space to accommodate file read");
    }
    file.seekg(0);
    file.read(buffer, f_size);
    file.close();
    return std::make_pair(buffer, f_size);
}

using namespace ZEROengine;

int main() {
    ZEROengine::ZEROengine app;

    // initializing graphical module
    VulkanGraphicalModule *vk_graphical_module = new VulkanGraphicalModule;
    vk_graphical_module->bindWindowContext(new SDL_VulkanWindowContext); // Vulkan Graphical Module is handling the pointer using a unique_ptr, 
                                                                        // so deallocation is depending on it.
    app.bindGraphicalModule(vk_graphical_module); // ZEROengine is handling the pointer using a unique_ptr, 
                                                  // so deallocation is depending on the engine
    app.init();

    auto frag_data = readFile(std::string(BINARY_PATH) + "/frag.spv");
    auto vert_data = readFile(std::string(BINARY_PATH) + "/vert.spv");

    VulkanGraphicsPipelineBuffer *pipeline_buffer{};
    vk_graphical_module->getGraphicsPipelineBuffer(&pipeline_buffer);
    VulkanGraphicsPipelineTemplate *pipeline_template{};
    vk_graphical_module->getForwardPassPipelineTemplate(&pipeline_template);

    VulkanContext *vulkan_context{};
    VulkanBasicScreenRenderer *screen_renderer{};
    VulkanRenderWindow *render_window{};
    VkDevice device{};
    VkRenderPass renderpass{};
    vk_graphical_module->getVulkanContext(&vulkan_context);
    vk_graphical_module->getScreenRenderer(&screen_renderer);
    screen_renderer->getRenderWindow(&render_window);
    render_window->getRenderPass(renderpass);
    vulkan_context->getDevice(device);

    std::vector<ShaderData> shader_data = { {vert_data, frag_data} };
    std::vector<std::size_t> pipeline_hash = 
    pipeline_buffer->requestGraphicsPipelines(
        device, 
        *pipeline_template,
        renderpass,
        shader_data);
    
    // freeing allocated buffers  
    delete vert_data.first;
    delete frag_data.first;
    
    try {
        app.run();
    } catch(const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}