#ifndef ZEROENGINE_VULKAN_PIPELINE_BUFFER_H
#define ZEROENGINE_VULKAN_PIPELINE_BUFFER_H

#include <vulkan/vulkan.hpp>

#include <memory>
#include <cstdint>
#include <vector>
#include <utility>
#include <unordered_map>

#include "zeroengine_vulkan/VulkanGraphicsPipelineTemplate.hpp"

namespace ZEROengine {
    /**
     * @brief VulkanGraphicsPipelineBuffer acts as a pool of loaded Graphics Pipeline, managing the allocation and destruction of such items.
     * The application is responsible for keeping the object present while any GPU operation is using it, and calling the cleanup function when it is no longer in use.
     * 
     */
    class VulkanGraphicsPipelineBuffer {
    private:
        std::unordered_map<std::size_t, VulkanGraphicsPipelineObject> pipeline_buffer;

    public:
        VulkanGraphicsPipelineBuffer();

        VulkanGraphicsPipelineObject getPipeline(std::size_t index) {
            return this->pipeline_buffer[index];
        }
        std::unordered_map<std::size_t, VulkanGraphicsPipelineObject>& getAllPipelines() {
            return this->pipeline_buffer;
        }
        std::vector<std::size_t> requestGraphicsPipelines(
            const VkDevice &device, 
            const VulkanGraphicsPipelineTemplate &pipeline_template, 
            const VkRenderPass &render_pass, 
            const std::vector<ShaderData> &data
        );

        void cleanup(VkDevice device) {
            for(auto &[key, pipeline] : pipeline_buffer) {
                vkDestroyPipeline(device, pipeline.vk_pipeline, nullptr);
            }
        }
    }; // class VulkanGraphicsPipelineBuffer

} // namespace ZEROengine

#endif // #ifndef VULKAN_PIPELINE_BUFFER_H