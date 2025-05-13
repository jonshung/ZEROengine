#include "zeroengine_vulkan/VulkanPipelineManager.hpp"

#include <numeric>
#include <utility>

#include <vulkan/vk_enum_string_helper.h>
#include "zeroengine_core/ZEROUtilities.hpp"

namespace ZEROengine {

    VulkanPipelineManager::VulkanPipelineManager() :
    m_pipeline_buffer{}
    {}

    // std::vector<std::size_t> VulkanPipelineManager::requestGraphicsPipelines(
    //     const VkDevice &device, 
    //     const VulkanGraphicsPipelineTemplate &pipeline_template, 
    //     const VkRenderPass &render_pass, 
    //     const std::vector<ShaderData> &data
    // ) {
    //     // pipeline layout cache
    //     std::size_t common_hash = hash_combine<VkRenderPass>((std::size_t) 0, m_render_pass);
    //     common_hash = hash_combine<VkPipelineRasterizationStateCreateInfo>(common_hash, pipeline_template.rasterization_state);
    //     common_hash = hash_combine<VkPipelineMultisampleStateCreateInfo>(common_hash, pipeline_template.multisample_state);
    //     for(std::size_t i = 0; i < pipeline_template.vertex_binding.size(); i++) {
    //         common_hash = hash_combine(common_hash, pipeline_template.vertex_binding[i]);
    //     }

    //     for(std::size_t i = 0; i < pipeline_template.vertex_attribute.size(); i++) {
    //         common_hash = hash_combine(common_hash, pipeline_template.vertex_attribute[i]);
    //     }
    //     std::vector<ShaderData> submitting_job;
    //     std::vector<std::size_t> ret_hash;
    //     for(const ShaderData &component : data) {
    //         std::size_t hash = hash_combine(common_hash, component.vertex_data.first, component.vertex_data.second);
    //         hash = hash_combine(hash, component.fragment_data.first, component.fragment_data.second);
    //         if(this->pipeline_buffer.find(hash) == this->pipeline_buffer.end()) {
    //             submitting_job.push_back(component);
    //             ret_hash.push_back(hash);
    //         }
    //     }

    //     std::vector<VkPipeline> produces = pipeline_template.produce(device, render_pass, submitting_job);
    //     for(std::size_t i = 0; i < ret_hash.size(); ++i) {
    //         this->pipeline_buffer[ret_hash[i]].vk_pipeline = produces[i];
    //         this->pipeline_buffer[ret_hash[i]].vk_pipeline_layout = pipeline_template.pipeline_layout;
    //     }
    //     return ret_hash;
    // }
} // namespace ZEROengine