#ifndef ZEROENGINE_VULKAN_BUFFER_H
#define ZEROENGINE_VULKAN_BUFFER_H

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

struct VulkanBuffer {
    VkBuffer vk_buffer;
    VmaAllocation vma_mem_alloc;
    VkDeviceSize mem_size; // actual allocated size, might be padded
    VkDeviceSize request_size; // requesting size
    void *p_buffer;
    bool mapped = false;

    /**
     * @brief A fast helper function to allocate a general-purpose buffer.
     * Application can opt to allocate buffers themselves or derive another VulkanBuffer class.
     * 
     * @param allocator 
     * @param size 
     * @param usage 
     * @param flags 
     */
    VmaAllocationInfo allocate( VmaAllocator &allocator, const VkDeviceSize &size, 
                                const VkBufferUsageFlags &usage, const VmaAllocationCreateFlags &flags,
                                const VmaMemoryUsage mem_usage_flag = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE) {
        request_size = size;
        
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
            &vk_buffer,      // VkBuffer
            &vma_mem_alloc,  // VmaAllocation
            &alloc_ret_info);   // VmaAllocationInfo
        if(flags & VMA_ALLOCATION_CREATE_MAPPED_BIT) {
            // mapped = true;     Persistent-mapped memory automatically get unmapped when destroying buffer.
            p_buffer = alloc_ret_info.pMappedData;
        }
        mem_size = alloc_ret_info.size;
        return alloc_ret_info;
    }

    /**
     * @brief In case the resources was not allocated by this structure dedicated procedure or is not explicitly set by the application, use this 
     * to get the resources size (Implicitly assumes all other resources are valid).
     * 
     * @param allocator 
     * @return VkDeviceSize 
     */
    VkDeviceSize getSize(VmaAllocator &allocator) {
        VmaAllocationInfo info;
        vmaGetAllocationInfo(allocator, vma_mem_alloc, &info);
        return info.size;
    }

    /**
     * @brief A fast helper function to map with the allocated resources.
     * Application can opt to map themselves or derive a VulkanBuffer class.
     * 
     * @param allocator 
     * @return void* 
     */
    void* map(VmaAllocator &allocator) {
        if(mapped) return p_buffer;
        vmaMapMemory(allocator, vma_mem_alloc, &p_buffer);
        mapped = true;
        return p_buffer;
    }

    /**
     * @brief A fast helper function to unmap with the allocated resources.
     * Application can opt to map themselves or derive a VulkanBuffer class.
     * 
     * @param allocator 
     * @return void* 
     */
    void unmap(VmaAllocator &allocator) {
        if(!mapped) return;
        vmaUnmapMemory(allocator, vma_mem_alloc);
        p_buffer = nullptr;
        mapped = false;
    }

    /**
     * @brief A fast helper function to deallocate all related resources.
     * Application can opt to deallocate buffers themselves or derive another VulkanBuffer class.
     * 
     * @param allocator 
     */
    void cleanup(VmaAllocator &allocator) {
        if(mapped) {
            unmap(allocator);
        }
        vmaDestroyBuffer(allocator, vk_buffer, vma_mem_alloc);
        this->p_buffer = nullptr;
    }
}; // struct VulkanBuffer

#endif // #ifndef ZEROENGINE_VULKAN_BUFFER_H