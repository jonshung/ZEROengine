#include <unordered_map>

#include "zeroengine_vulkan/VulkanQueueManager.hpp"
#include "zeroengine_vulkan/VulkanDefines.hpp"

namespace ZEROengine {
    
    void VulkanQueueManager::init(const VkDevice &vk_device) {
        for(const auto &[q, queue_family] : m_queue_indices) {
            m_queue[q] = VulkanQueueInfo();
            m_queue[q].queueFamilyIndex = queue_family;
            vkGetDeviceQueue(vk_device, queue_family, 0, &m_queue[q].queue);
        }
    }

    std::unordered_map<VkQueueFlags, uint32_t> VulkanQueueManager::queryQueueFamily(const VkPhysicalDevice &phys_device, const std::vector<VkQueueFlags> &query) {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, queue_families.data());
        
        std::unordered_map<VkQueueFlags, uint32_t> q_indices;

        uint32_t i = 0;
        for(const auto& family : queue_families) {
            for(const uint32_t& q : query) {
                if(q_indices.count(q) > 0) continue;
                if(family.queueFlags & q) {
                    q_indices[q] = i;
                }
            }
            i++;
        }
        return q_indices;
    }

    std::vector<VkDeviceQueueCreateInfo> VulkanQueueManager::queryQueueCreation(const VkPhysicalDevice& phys_device) {
        const std::vector<VkQueueFlags> const_requesting_queues = { VK_QUEUE_GRAPHICS_BIT };
        
        m_queue_indices = queryQueueFamily(phys_device, const_requesting_queues);
        for(const VkQueueFlags& q : const_requesting_queues) {
            if(m_queue_indices.count(q) == 0) {
                ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "Missing queue " + q);
            }
        }
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
        float queuePriority = 1.0f;
        
        for (const auto &[q, queue_family] : m_queue_indices) {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family;
            queue_create_info.queueCount = 1;
            queue_create_info.pQueuePriorities = &queuePriority;
            queue_create_infos.push_back(queue_create_info);
        }
        return queue_create_infos;
    }

    bool VulkanQueueManager::getQueueInfo(const VkQueueFlags &queue_flags, VulkanQueueInfo& queue_info) {
        if(m_queue.count(queue_flags) == 0) {
            return false;
        }
        queue_info = m_queue[queue_flags];
        return true;
    }
} // namespace ZEROengine