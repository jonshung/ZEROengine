#include "zeroengine_graphical/GPUResource.hpp"

namespace ZEROengine {
    void* GPUBuffer::getBufferHandle() {
        return m_buffer_handle;
    }

    void GPUBuffer::setBufferHandle(void* handle) {
        m_buffer_handle = handle;
    }

    void* GPUBuffer::getBufferMapped() {
        return m_buffer_mapped;
    }

    void GPUBuffer::cleanup(){
        m_buffer_mapped = nullptr;
        m_buffer_handle = nullptr;
    }
} // namespace ZEROengine