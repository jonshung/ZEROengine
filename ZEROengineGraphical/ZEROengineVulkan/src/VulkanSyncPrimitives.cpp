#include "zeroengine_vulkan/VulkanSyncPrimitives.hpp"
#include "zeroengine_vulkan/VulkanDefines.hpp"

namespace ZEROengine {
    VulkanSyncPrimitives::VulkanSyncPrimitives(const VkDevice& vk_device) {
        VkSemaphore semaphore_handle;
        VkSemaphoreCreateInfo semaphore_info{};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphore_info.pNext = nullptr;
        semaphore_info.flags = 0;
        ZERO_VK_CHECK_EXCEPT(vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &semaphore_handle));
        m_api_semaphore_handle = static_cast<void*>(semaphore_handle);

        VkFence fence_handle;
        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.pNext = nullptr;
        //fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        ZERO_VK_CHECK_EXCEPT(vkCreateFence(vk_device, &fence_info, nullptr, &fence_handle));
        m_api_fence_handle = static_cast<void*>(fence_handle);

        m_vk_device = vk_device;
    }

    void VulkanSyncPrimitives::waitOnFence() {
        VkFence fence_handle = static_cast<VkFence>(m_api_fence_handle);
        vkWaitForFences(m_vk_device, 1, &fence_handle, VK_TRUE, 1e9); 
    }

    bool VulkanSyncPrimitives::getFenceStatus() {
        VkFence fence_handle = static_cast<VkFence>(m_api_fence_handle);
        return vkGetFenceStatus(m_vk_device, fence_handle) == VK_SUCCESS;
    }

    void VulkanSyncPrimitives::releaseFence() {
        VkFence fence_handle = static_cast<VkFence>(m_api_fence_handle);
        vkResetFences(m_vk_device, 1, &fence_handle);
    }

    void* VulkanSyncPrimitives::getSemaphore() {
        return m_api_semaphore_handle;
    }

} // namespace ZEROengine