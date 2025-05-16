#include <memory>
#include <cstdint>

#include "vulkan/vulkan.hpp"
#include "zeroengine_vulkan/VulkanContext.hpp"
#include "zeroengine_vulkan/VulkanDefines.hpp"

namespace ZEROengine {
    VulkanGraphicalContext::VulkanGraphicalContext(const VkDevice &vk_device, const uint32_t &queue_family) : 
        m_primary_command_buffer{},
        m_primary_command_pool{},
        m_vk_device{vk_device},
        m_queue_family{queue_family}
    {
        init();
    }

    void VulkanGraphicalContext::init() {
        VkCommandPoolCreateInfo pool_create_info{};
        pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_create_info.pNext = nullptr;
        pool_create_info.queueFamilyIndex = m_queue_family;
        ZERO_VK_CHECK_EXCEPT(vkCreateCommandPool(m_vk_device, &pool_create_info, nullptr, &m_primary_command_pool));

        VkCommandBufferAllocateInfo primary_cmd_buffer_allocation{};
        primary_cmd_buffer_allocation.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        primary_cmd_buffer_allocation.pNext = nullptr;
        ZERO_VK_CHECK_EXCEPT(vkAllocateCommandBuffers(m_vk_device, &primary_cmd_buffer_allocation, &m_primary_command_buffer));
    }

    // std::weak_ptr<GraphicalCommandBuffer> VulkanGraphicalContext::allocateCommandBuffer() {

    // }

    // std::weak_ptr<GraphicalCommandBuffer> VulkanGraphicalContext::getCommandBuffer() {
        
    // }

    // size_t VulkanGraphicalContext::countCommandBuffers() const {
        
    // }
    
    void VulkanGraphicalContext::beginRecording() {
    }

    // void VulkanGraphicalContext::endRecording() {
        
    // }
    
    // void VulkanGraphicalContext::cleanup() {

    // }
} // namespace ZEROengine