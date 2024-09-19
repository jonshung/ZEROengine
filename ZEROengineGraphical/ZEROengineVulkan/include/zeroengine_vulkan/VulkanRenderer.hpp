#ifndef ZEROENGINE_VULKAN_RENDERER_H
#define ZEROENGINE_VULKAN_RENDERER_H

#include <vulkan/vulkan.hpp>

#include <cstdint>
#include <vector>
#include <utility>
#include <functional>

#include "zeroengine_vulkan/VulkanContext.hpp"
#include "zeroengine_vulkan/VulkanPipelineBuffer.hpp"
#include "zeroengine_core/Renderer.hpp"
#include "zeroengine_core/ZEROengineDefines.hpp"
#include "zeroengine_vulkan/VulkanCommand.hpp"

namespace ZEROengine {
    /**
     * @brief General purpose Renderer abstraction.
     * A Renderer implements all necessary logics to record secondary command buffers to submit to a RenderContext for execution
     * 
     */
    class VulkanRenderer : Renderer {
    protected:
        VkCommandPool vk_render_cmd_pool;
        VulkanContext *vulkan_context;

    // initialization and cleanup procedures
    public:
        virtual ZEROResult initRenderer() override { return { ZERO_SUCCESS, "" }; }
        /**
         * @brief Before usage, any Vulkan Renderer should be initialized by calling this function.
         * 
         * @param settings 
         */
        virtual ZEROResult initVulkanRenderer(VulkanContext *vulkan_context);
        virtual void cleanup() {
            VkDevice device;
            this->vulkan_context->getDevice(device);
            vkDestroyCommandPool(device, this->vk_render_cmd_pool, nullptr);
        }

    public:
        virtual void reset() {
            VkDevice device;
            this->vulkan_context->getDevice(device);
            vkResetCommandPool(device, this->vk_render_cmd_pool, 0);
        }
        virtual void begin() = 0;
        virtual void configureViewportAndScissor(VkExtent2D &extent) = 0;
        // currently directly passing the graphics pipeline buffer to every renderer via here. Should be passing a structure of
        // world objects material and geometry informations in the future
        virtual void draw(VulkanGraphicsPipelineBuffer *const g_pipeline_buffer) = 0;
        virtual void end() = 0;
        virtual ZEROResult record(VulkanGraphicsPipelineBuffer *const pipeline_buffer, std::vector<VulkanGraphicsRecordingInfo> &ret) = 0;

    }; // class VulkanRenderer

} // namespace ZEROengine

#endif // #ifndef VULKAN_RENDERER_H