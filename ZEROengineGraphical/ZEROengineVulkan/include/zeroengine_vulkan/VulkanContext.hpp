#ifndef ZEROENGINE_VULKANCONTEXT_H
#define ZEROENGINE_VULKANCONTEXT_H

#include "zeroengine_graphical/GraphicalContext.hpp"
#include "vulkan/vulkan.hpp"

namespace ZEROengine {
    class VulkanGraphicalContext : public GraphicalContext {
    private:
        VkCommandBuffer m_command_buffer;
        VkCommandPool m_command_pool;
    public:
        void init() override final;
        void cleanup() override final;
        
        void beginRecording() override final;
        void endRecording() override final;

        void bindPipeline() override final;
        void bindVertex() override final;
    }; // class VulkanGraphicalContext
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKANCONTEXT_H