#ifndef ZEROENGINE_GPURESOURCE_H
#define ZEROENGINE_GPURESOURCE_H

#include <memory>
#include <cstdint>

#include "zeroengine_graphical/GPUSyncPrimitives.hpp"
#include "zeroengine_graphical/GPUDefines.hpp"

namespace ZEROengine {
    class GPUResource {
    protected:
        std::shared_ptr<GPUSyncPrimitive> m_sync_resource_ready;
        std::shared_ptr<GPUSyncPrimitive> m_sync_resource_work_done;

    public:
        virtual std::weak_ptr<GPUSyncPrimitive> getResourceReadySync() = 0;
        virtual std::weak_ptr<GPUSyncPrimitive> getResourceWorkDoneSync() = 0;

        virtual void cleanup() = 0;
    }; // class GPUResource

    enum GPUBufferUsageBits {
        ZERO_BUFFER_USAGE_TRANSFER_SRC = 0x00000001,
        ZERO_BUFFER_USAGE_TRANSFER_DST = 0x00000002,
        ZERO_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER = 0x00000004,
        ZERO_BUFFER_USAGE_STORAGE_TEXEL_BUFFER = 0x00000008,
        ZERO_BUFFER_USAGE_UNIFORM_BUFFER = 0x00000010,
        ZERO_BUFFER_USAGE_STORAGE_BUFFER = 0x00000020,
        ZERO_BUFFER_USAGE_INDEX_BUFFER = 0x00000040,
        ZERO_BUFFER_USAGE_VERTEX_BUFFER = 0x00000080,
        ZERO_BUFFER_USAGE_INDIRECT_BUFFER = 0x00000100,
        ZERO_BUFFER_USAGE_SHADER_DEVICE_ADDRESS = 0x00020000,
        ZERO_BUFFER_USAGE_VIDEO_DECODE_SRC_KHR = 0x00002000,
        ZERO_BUFFER_USAGE_VIDEO_DECODE_DST_KHR = 0x00004000,
        ZERO_BUFFER_USAGE_TRANSFORM_FEEDBACK_BUFFER_EXT = 0x00000800,
        ZERO_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_EXT = 0x00001000,
        ZERO_BUFFER_USAGE_CONDITIONAL_RENDERING_EXT = 0x00000200,
        ZERO_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_KHR = 0x00080000,
        ZERO_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_KHR = 0x00100000,
        ZERO_BUFFER_USAGE_SHADER_BINDING_TABLE_KHR = 0x00000400,
    };
    typedef GPUFlags GPUBufferUsageFlags;

    struct GPUBufferDescription{
        uint64_t stride;
        uint64_t size;
        uint32_t elements_count;

        GPUBufferUsageFlags m_usage;
    };

    class GPUBuffer : GPUResource {
    protected:
        void* m_buffer_handle;
        void* m_buffer_mapped;

        GPUBufferDescription m_buffer_description;

    public:
        virtual uint64_t getSize() const = 0;
        virtual uint64_t getStride() const = 0;
        virtual uint64_t getElementsCount() const = 0;
        
        virtual void* getBufferHandle() = 0;
        virtual void setBufferHandle(void* handle) = 0;

        virtual void* getBufferMapped() = 0;

        virtual void cleanup() override;
    }; // class GPUBuffer

    class GPUImage : public GPUResource {
        
    }; // class GPUImage
};

#endif // #ifndef ZEROENGINE_GPURESOURCE_H