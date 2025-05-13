#ifndef ZEROENGINE_VULKANPIPELINEMANAGER_H
#define ZEROENGINE_VULKANPIPELINEMANAGER_H

#include "vulkan/vulkan.hpp"

#include "zeroengine_vulkan/VulkanPipelineObject.hpp"

#include <memory>
#include <cstdint>
#include <vector>
#include <utility>
#include <unordered_map>

namespace ZEROengine {
    /**
     * @brief VulkanGraphicsPipelineBuffer acts as a pool of loaded Pipeline, managing the allocation and destruction of such items.
     * The application is responsible for keeping the object present while any GPU operation is using it, and calling the cleanup function when it is no longer in use.
     * 
     */
    class VulkanPipelineManager {
    private:
        std::unordered_map<std::size_t, VulkanPipelineObject> m_pipeline_buffer;

    public:
        VulkanPipelineManager();

        VulkanPipelineObject getPipeline(std::size_t index) {
            return this->m_pipeline_buffer[index];
        }
        std::unordered_map<std::size_t, VulkanPipelineObject>& getAllPipelines() {
            return this->m_pipeline_buffer;
        }

        void cleanup(VkDevice device) {
            for(auto &[key, pipeline] : this->m_pipeline_buffer) {
                vkDestroyPipeline(device, pipeline.vk_pipeline, nullptr);
            }
        }
    }; // class VulkanGraphicsPipelineBuffer

} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKAN_PIPELINE_MANAGER_H