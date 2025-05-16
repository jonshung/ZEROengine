#ifndef ZEROENGINE_VULKANSYNCPRIMITIVES_H
#define ZEROENGINE_VULKANSYNCPRIMITIVES_H

#include "vulkan/vulkan.hpp"
#include "zeroengine_graphical/GPUSyncPrimitives.hpp"

namespace ZEROengine {
    class VulkanSyncPrimitives : public GPUSyncPrimitive {
    private:
        VkDevice m_vk_device;
        
    public:
        VulkanSyncPrimitives(const VkDevice &vk_device);

        void waitOnFence() override final;
        bool getFenceStatus() override final;
        void releaseFence() override final;

        void* getSemaphore() override final;
    }; // class VulkanSyncPrimitives
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKANSYNCPRIMITIVES_H