#ifndef ZEROENGINE_VULKANQUEUEMANAGER_H
#define ZEROENGINE_VULKANQUEUEMANAGER_H

#include <unordered_map>
#include <string>
#include <vector>
#include <cstdint>

#include "vulkan/vulkan.hpp"
#include "zeroengine_vulkan/VulkanDefines.hpp"

namespace ZEROengine {
    class VulkanQueueManager {
    private:
        std::unordered_map<VkQueueFlags, VulkanQueueInfo> m_queue;
        std::unordered_map<VkQueueFlags, uint32_t> m_queue_indices;
        VkDevice m_vk_device;

        std::unordered_map<VkQueueFlags, uint32_t> queryQueueFamily(const VkPhysicalDevice &phys_device, const std::vector<VkQueueFlags> &query);

    public:
        VulkanQueueManager();
        void init(const VkDevice &vk_device);

        std::vector<VkDeviceQueueCreateInfo> queryQueueCreation(const VkPhysicalDevice &phys_device);
        bool getQueueInfo(const VkQueueFlags &queue_flags, VulkanQueueInfo &queue_info);
    }; // class VulkanQueueManager
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKANQUEUEMANAGER_H