#ifndef ZEROENGINE_GPUCOMMANDBUFFER_H
#define ZEROENGINE_GPUCOMMANDBUFFER_H

#include <memory>

namespace ZEROengine {
    class GPUCommandBuffer {
    public:
        virtual void init() = 0;
        virtual void cleanup() = 0;

    }; // class GPUCommandBuffer

    class GraphicalCommandBuffer : public GPUCommandBuffer {
    public:
        virtual void bindVertex() = 0;
        virtual void bindPipeline() = 0;
        virtual void draw() = 0;
    }; // class GraphicalCommandBuffer
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_GRAPHICALCOMMANDBUFFER_H