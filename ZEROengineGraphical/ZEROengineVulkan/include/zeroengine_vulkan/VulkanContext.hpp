#ifndef ZEROENGINE_VULKANCONTEXT_H
#define ZEROENGINE_VULKANCONTEXT_H

#include <memory>
#include <cstdint>

#include "zeroengine_graphical/GPUContext.hpp"
#include "zeroengine_graphical/GPUCommandBuffer.hpp"
#include "vulkan/vulkan.hpp"

namespace ZEROengine {
    class VulkanGraphicalContext : public GraphicalContext {
    private:
        VkCommandBuffer m_primary_command_buffer;
        VkCommandPool m_primary_command_pool;
        VkDevice m_vk_device;

        const uint32_t m_queue_family;

    public:
        VulkanGraphicalContext(const VkDevice& vk_device, const uint32_t &queue_family);
        void init() override final;
        void cleanup() override final;

        std::weak_ptr<GraphicalCommandBuffer> allocateCommandBuffer() override final;
        std::weak_ptr<GraphicalCommandBuffer> getCommandBuffer() override final;
        size_t countCommandBuffers() const override final;
        
        void beginRecording() override final;
        void endRecording() override final;
    
    // prohibited methods
    private:
        VulkanGraphicalContext();
    }; // class VulkanGraphicalContext

    // class VulkanComputeContext : public ComputeContext {
    // private:
    //     VkCommandBuffer m_primary_command_buffer;
    //     VkCommandPool m_primary_command_pool;

    // public:
    //     void init() override final;
    //     void cleanup() override final;
        
    //     void beginRecording() override final;
    //     void endRecording() override final;
    // }; // class VulkanGraphicalContext
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKANCONTEXT_H