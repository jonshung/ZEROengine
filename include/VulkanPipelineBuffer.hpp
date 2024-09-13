#ifndef ZEROENGINE_VULKAN_PIPELINE_BUFFER_H
#define ZEROENGINE_VULKAN_PIPELINE_BUFFER_H

#include <vulkan/vulkan.hpp>

#include <memory>
#include <cstdint>
#include <vector>
#include <utility>
#include <unordered_map>

struct ShaderData {
    std::pair<char*, std::size_t> vertex_data;
    std::pair<char*, std::size_t> fragment_data;
};

// debug dynamic viewport state
const std::vector<VkDynamicState> dynamic_states = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

/**
 * @brief A VulkanGraphicsPipelineTemplate provides necessary information to form a family of Graphics Pipeline, allowing the reusage of
 * multiple render stage create info with different shader program to create VkPipeline. It also holds the VkPipelineLayout that is used
 * to compile Pipeline, so application should be responsible for this class of object's lifetime while any GPU operation is using it, and cleanup()
 * when it is lo longer in use.
 * 
 */
class VulkanGraphicsPipelineTemplate {
private:
    VkPipelineLayout pipeline_layout;

public:
    VulkanGraphicsPipelineTemplate();
    std::vector<VkVertexInputBindingDescription> vertex_binding;
    std::vector<VkVertexInputAttributeDescription> vertex_attribute;
    VkPipelineRasterizationStateCreateInfo rasterization_state;
    VkPipelineMultisampleStateCreateInfo multisample_state;

    // TODO: multi sampling support
    
    virtual void createGraphicsPipelinesLayout(VkDevice &device);

public:
    virtual VkShaderModule createShaderModule(VkDevice &device, const char* data, const std::size_t &data_size);
    virtual std::vector<VkPipeline> produce(VkDevice &device, VkRenderPass &render_pass, std::vector<ShaderData> &shaders);
    void cleanup(VkDevice &device) {
        vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
    }
};

/**
 * @brief VulkanGraphicsPipelineBuffer acts as a pool of loaded Graphics Pipeline, managing the allocation and destruction of such items.
 * The application is responsible for keeping the object present while any GPU operation is using it, and calling the cleanup function when it is no longer in use.
 * 
 */
class VulkanGraphicsPipelineBuffer {
private:
    std::unordered_map<std::size_t, VkPipeline> pipeline_buffer;

public:
    VkPipeline getPipeline(std::size_t index) {
        return this->pipeline_buffer[index];
    }
    std::unordered_map<std::size_t, VkPipeline>& getAllPipelines() {
        return this->pipeline_buffer;
    }
    std::vector<std::size_t> requestGraphicsPipelines(
        VkDevice &device, 
        VulkanGraphicsPipelineTemplate &pipeline_template, 
        VkRenderPass &render_pass, 
        std::vector<ShaderData> &data
    );

    void cleanup(VkDevice device) {
        for(auto &[key, pipeline] : pipeline_buffer) {
            vkDestroyPipeline(device, pipeline, nullptr);
        }
    }
};

#endif // #ifndef VULKAN_PIPELINE_BUFFER_H