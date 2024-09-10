#ifndef VULKAN_PIPELINE_BUFFER_H
#define VULKAN_PIPELINE_BUFFER_H

#include <vulkan/vulkan.hpp>

#include <memory>
#include <cstdint>
#include <vector>
#include <tuple>

struct ShaderData {
    std::tuple<char*, size_t> vertex_data;
    std::tuple<char*, size_t> fragment_data;
};

// debug dynamic viewport state
const std::vector<VkDynamicState> dynamic_states = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

class VulkanPipelineBuffer {
private:
    std::shared_ptr<std::vector<VkPipeline>> pipeline_buffer;
    std::shared_ptr<std::vector<VkPipelineLayout>> pipeline_layout;
    VkRenderPass render_pass;

public:
    VulkanPipelineBuffer(VkRenderPass _render_pass) {
        pipeline_buffer = std::make_shared<std::vector<VkPipeline>>();
        pipeline_layout = std::make_shared<std::vector<VkPipelineLayout>>();
        this->render_pass = _render_pass;
    }

    VkShaderModule createShaderModule(VkDevice device, const char* data, const size_t &data_size);
    std::vector<size_t> createGraphicsPipelines(VkDevice device, VkPipelineLayout pipeline_layout, VkRenderPass render_pass,std::vector<ShaderData> shaders);
    size_t createGraphicsPipelinesLayout(VkDevice);
    VkPipeline getPipeline(size_t index) {
        return (*this->pipeline_buffer)[index];
    }
    VkPipelineLayout getPipelineLayout(size_t index) {
        return (*this->pipeline_layout)[index];
    }

    void cleanup(VkDevice device) {
        for(auto &pipeline : (*pipeline_buffer)) {
            vkDestroyPipeline(device, pipeline, nullptr);
        }
        for(auto &pipeline_layout : (*pipeline_layout)) {
            vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
        }
    }

    VulkanPipelineBuffer() = delete;
};

#endif // #ifndef VULKAN_PIPELINE_BUFFER_H