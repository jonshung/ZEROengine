#ifndef ZEROENGINE_VULKAN_HARDWARE_BUFFER_H
#define ZEROENGINE_VULKAN_HARDWARE_BUFFER_H

#include "zeroengine_graphical/HardwareBuffer.hpp"

#include "vulkan/vulkan.hpp"
#include <vk_mem_alloc.h>

namespace ZEROengine {
    class VulkanHardwareBuffer : public HardwareBuffer {
    public:
        VkBuffer m_vk_buffer;
        VmaAllocation m_vma_mem_alloc;
        VkDeviceSize m_mem_size; // actual allocated size, might be padded
        VkDeviceSize m_request_size; // requesting size
        bool m_mapped = false;

        VulkanHardwareBuffer() : HardwareBuffer() {}
        ~VulkanHardwareBuffer() {
            // None
        }
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

        /**
         * @brief In case the resources was not allocated by this structure dedicated procedure or is not explicitly set by the application, use this 
         * to get the resources size (Implicitly assumes all other resources are valid).
         * 
         * @param allocator 
         * @return VkDeviceSize 
         */
        VkDeviceSize getSize(VmaAllocator &allocator) {
            VmaAllocationInfo info;
            vmaGetAllocationInfo(allocator, m_vma_mem_alloc, &info);
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
            if(m_mapped) return m_p_buffer;
            vmaMapMemory(allocator, m_vma_mem_alloc, &m_p_buffer);
            m_mapped = true;
            return m_p_buffer;
        }

        /**
         * @brief A fast helper function to unmap with the allocated resources.
         * Application can opt to map themselves or derive a VulkanBuffer class.
         * 
         * @param allocator 
         * @return void* 
         */
        void unmap(VmaAllocator &allocator) {
            if(!m_mapped) return;
            vmaUnmapMemory(allocator, m_vma_mem_alloc);
            m_p_buffer = nullptr;
            m_mapped = false;
        }

        /**
         * @brief A fast helper function to deallocate all related resources.
         * Application can opt to deallocate buffers themselves or derive another VulkanBuffer class.
         * 
         * @param allocator 
         */
        void cleanup(VmaAllocator &allocator) {
            if(m_mapped) {
                unmap(allocator);
            }
            vmaDestroyBuffer(allocator, m_vk_buffer, m_vma_mem_alloc);
            this->m_p_buffer = nullptr;
        }
    }; // class VulkanHardwareBuffer
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKAN_BUFFER_H