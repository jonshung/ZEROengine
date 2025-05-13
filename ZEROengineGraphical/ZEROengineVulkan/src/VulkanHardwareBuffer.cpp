#include "zeroengine_vulkan/VulkanHardwareBuffer.hpp"

namespace ZEROengine {

    VulkanHardwareBuffer::VulkanHardwareBuffer() : HardwareBuffer() {}
    VulkanHardwareBuffer::~VulkanHardwareBuffer() {
        // None
    }

    void VulkanHardwareBuffer::cleanup(VmaAllocator &allocator) {
        if(m_mapped) {
            unmap(allocator);
        }
        vmaDestroyBuffer(allocator, m_vk_buffer, m_vma_mem_alloc);
        this->m_p_buffer = nullptr;
    }

    VmaAllocationInfo VulkanHardwareBuffer::allocate( VmaAllocator &allocator, const VkDeviceSize &size, 
                                const VkBufferUsageFlags &usage, const VmaAllocationCreateFlags &flags,
                                const VmaMemoryUsage mem_usage_flag = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE) {
        m_request_size = size;
        
        VkBufferCreateInfo buffer_info{};
        buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buffer_info.size = size;
        buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_info.pQueueFamilyIndices = nullptr;
        buffer_info.queueFamilyIndexCount = 0;
        buffer_info.usage = usage;

        VmaAllocationCreateInfo alloc_info{};
        alloc_info.usage = mem_usage_flag;
        alloc_info.flags = flags;
        
        VmaAllocationInfo alloc_ret_info;
        vmaCreateBuffer(
            allocator,    // VmaAllocator
            &buffer_info,       // VkBufferCreateInfo
            &alloc_info,        // VmaAllocationCreateInfo
            &m_vk_buffer,      // VkBuffer
            &m_vma_mem_alloc,  // VmaAllocation
            &alloc_ret_info);   // VmaAllocationInfo
        if(flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) {
            // mapped = true;     Persistent-mapped memory automatically get unmapped when destroying buffer.
            m_p_buffer = alloc_ret_info.pMappedData;
        }
        m_mem_size = alloc_ret_info.size;
        return alloc_ret_info;
    }

    VkDeviceSize VulkanHardwareBuffer::getSize(VmaAllocator &allocator) {
        VmaAllocationInfo info;
        vmaGetAllocationInfo(allocator, m_vma_mem_alloc, &info);
        return info.size;
    }

    void* VulkanHardwareBuffer::map(VmaAllocator &allocator) {
        if(m_mapped) return m_p_buffer;
        vmaMapMemory(allocator, m_vma_mem_alloc, &m_p_buffer);
        m_mapped = true;
        return m_p_buffer;
    }

    void VulkanHardwareBuffer::unmap(VmaAllocator &allocator) {
        if(!m_mapped) return;
        vmaUnmapMemory(allocator, m_vma_mem_alloc);
        m_p_buffer = nullptr;
        m_mapped = false;
    }
} //namespace ZEROengine