#ifndef ZEROENGINE_VULKANCOMMANDBUFFER_H
#define ZEROENGINE_VULKANCOMMANDBUFFER_H

#include "zeroengine_graphical/GraphicalCommandBuffer.hpp"
#include "vulkan/vulkan.hpp"

namespace ZEROengine {
    class VulkanCommandBuffer : GraphicalCommandBuffer {
    private:
        VkCommandBuffer m_api_handle;
        VkCommandBufferLevel m_level;
    public:
        void init() override final;
        void cleanup() override final;

        void bindVertex() override final;
        void bindPipeline() override final;
        void draw() override final;
    }; // class VulkanCommandBuffer
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKANCOMMANDBUFFER_H