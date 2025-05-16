#ifndef ZEROENGINE_VULKANRESOURCE_H
#define ZERONEGINE_VULKANRESOURCE_H

#include <cstdint>

#include "zeroengine_graphical/GPUResource.hpp"
#include "vk_mem_alloc.h"

namespace ZEROengine {
    class VulkanBuffer : GPUBuffer {
    private:
        VmaAllocator m_vma_alloc;
        VmaAllocation m_allocation_info;

    public:
        VulkanBuffer(const VmaAllocator &vma_alloc, const GPUBufferDescription &buffer_description);
        ~VulkanBuffer();

        uint64_t getSize() const override final;
        uint64_t getStride() const override final;
        uint64_t getElementsCount() const override final;

        VkBufferUsageFlags translateBufferUsage();

        void allocate();

        void cleanup();

    }; // class VulkanBuffer
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKANRESOURCE_H