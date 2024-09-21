#ifndef ZEROENGINE_VULKAN_GRAPHICS_PIPELINE_TEMPLATE_H
#define ZEROENGINE_VULKAN_GRAPHICS_PIPELINE_TEMPLATE_H

#include <vulkan/vulkan.hpp>

#include <vector>
#include <utility>
#include <cstdint>

#include "zeroengine_vulkan/VulkanGraphicsPipelineObject.hpp"

namespace ZEROengine {
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
    public:
        VkPipelineLayout pipeline_layout;

        VkDescriptorSetLayout descriptor_layout;
        std::vector<VkDescriptorSetLayoutBinding> descriptor_layout_bindings;

    public:
        std::vector<VkVertexInputBindingDescription> vertex_binding;
        std::vector<VkVertexInputAttributeDescription> vertex_attribute;
        VkPipelineRasterizationStateCreateInfo rasterization_state;
        VkPipelineMultisampleStateCreateInfo multisample_state;
        
        // TODO: multi sampling support

        
        VulkanGraphicsPipelineTemplate();
        virtual void createGraphicsPipelinesLayout(const VkDevice &device);

    public:
        virtual VkShaderModule createShaderModule(const VkDevice &device, const char* data, const std::size_t &data_size) const;
        virtual std::vector<VkPipeline> produce(const VkDevice &device, const VkRenderPass &render_pass, const std::vector<ShaderData> &shaders) const;
        void cleanup(const VkDevice &device) {
            vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
            vkDestroyDescriptorSetLayout(device, this->descriptor_layout, nullptr);
        }
    }; // class VulkanGraphicsPipelineTemplate
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKAN_GRAPHICS_PIPELINE_TEMPLATE_H