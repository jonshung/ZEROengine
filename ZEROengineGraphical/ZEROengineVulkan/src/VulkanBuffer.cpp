#include "zeroengine_vulkan/VulkanResource.hpp"
#include "zeroengine_vulkan/VulkanDefines.hpp"
#include "vk_mem_alloc.h"

namespace ZEROengine {
    VulkanBuffer::VulkanBuffer(const VmaAllocator &vma_alloc, const GPUBufferDescription &buffer_description) : GPUBuffer() {
        m_vma_alloc = vma_alloc;
        m_buffer_description = buffer_description;
        allocate();
    }

    VulkanBuffer::~VulkanBuffer() {
        cleanup();
    }
    
    VkBufferUsageFlags VulkanBuffer::translateBufferUsage() {
        return m_buffer_description.m_usage;
    }

    void VulkanBuffer::allocate() {
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = m_buffer_description.size;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_info.pQueueFamilyIndices = nullptr;
        buffer_info.queueFamilyIndexCount = 0;
        buffer_info.usage = translateBufferUsage();

        VmaAllocationCreateInfo alloc_info{};
        alloc_info.usage = VMA_MEMORY_USAGE_AUTO;
        alloc_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        
        VmaAllocationInfo alloc_ret_info;
        VkBuffer buffer_handle = VK_NULL_HANDLE;
        ZERO_VK_CHECK_EXCEPT(vmaCreateBuffer(
            m_vma_alloc,    // VmaAllocator
            &buffer_info,       // VkBufferCreateInfo
            &alloc_info,        // VmaAllocationCreateInfo
            &buffer_handle,      // VkBuffer
            &m_allocation_info,  // VmaAllocation
            &alloc_ret_info));   // VmaAllocationInfo
        m_buffer_mapped = alloc_ret_info.pMappedData;
    }

    uint64_t VulkanBuffer::getSize() const {
        return m_buffer_description.size;
    }

    uint64_t VulkanBuffer::getStride() const {
        return m_buffer_description.stride;
    }

    uint64_t VulkanBuffer::getElementsCount() const {
        return m_buffer_description.elements_count;
    }
        
    void VulkanBuffer::cleanup() {
        VkBuffer buffer_handle = static_cast<VkBuffer>(m_buffer_handle);
        vmaDestroyBuffer(m_vma_alloc, buffer_handle, m_allocation_info);
        GPUBuffer::cleanup();
    }
} //namespace ZEROengine