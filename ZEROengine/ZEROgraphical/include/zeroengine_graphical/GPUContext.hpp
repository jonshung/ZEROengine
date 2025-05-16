#ifndef ZEROENGINE_GPUCONTEXT_H
#define ZEROENGINE_GPUCONTEXT_H

#include <memory>
#include <list>
#include <cstdint>

#include "zeroengine_graphical/GPUCommandBuffer.hpp"

namespace ZEROengine {
    class GPUContext {
    private:
        std::list<std::shared_ptr<GPUCommandBuffer>> m_command_buffers;

    public:
        virtual void init() = 0;
        virtual void cleanup() = 0;

        virtual std::weak_ptr<GraphicalCommandBuffer> allocateCommandBuffer() = 0;
        virtual std::weak_ptr<GraphicalCommandBuffer> getCommandBuffer() = 0;
        virtual size_t countCommandBuffers() const = 0;

        virtual void beginRecording() = 0;
        virtual void endRecording() = 0;
    }; // class GPUContext

    class GraphicalContext : public GPUContext {
    private:
        virtual void beginRecording() override = 0;
        virtual void endRecording() override = 0;
    }; // class GraphicalContext
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_GPUCONTEXT_H