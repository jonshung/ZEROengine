#ifndef ZEROENGINE_VULKANHARDWAREBUFFER_H
#define ZEROENGINE_VULKANHARDWAREBUFFER_H

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

        VulkanHardwareBuffer();
        ~VulkanHardwareBuffer();
        
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
                                    const VmaMemoryUsage mem_usage_flag = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE);

        /**
         * @brief In case the resources was not allocated by this structure dedicated procedure or is not explicitly set by the application, use this 
         * to get the resources size (Implicitly assumes all other resources are valid).
         * 
         * @param allocator 
         * @return VkDeviceSize 
         */
        VkDeviceSize getSize(VmaAllocator &allocator) const;

        /**
         * @brief A fast helper function to map with the allocated resources.
         * Application can opt to map themselves or derive a VulkanBuffer class.
         * 
         * @param allocator 
         * @return void* 
         */
        void* map(VmaAllocator &allocator);

        /**
         * @brief A fast helper function to unmap with the allocated resources.
         * Application can opt to map themselves or derive a VulkanBuffer class.
         * 
         * @param allocator 
         * @return void* 
         */
        void unmap(VmaAllocator &allocator);

        /**
         * @brief A fast helper function to deallocate all related resources.
         * Application can opt to deallocate buffers themselves or derive another VulkanBuffer class.
         * 
         * @param allocator 
         */
        void cleanup(VmaAllocator &allocator);
    }; // class VulkanHardwareBuffer
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKANHARDWAREBUFFER_H