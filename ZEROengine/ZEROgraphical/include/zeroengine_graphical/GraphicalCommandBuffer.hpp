#ifndef ZEROENGINE_GRAPHICALCOMMANDBUFFER_H
#define ZEROENGINE_GRAPHICALCOMMANDBUFFER_H

#include <memory>

namespace ZEROengine {
    class CommandBuffer {
    public:
        virtual void init() = 0;
        virtual void cleanup() = 0;

    }; // class Context

    class GraphicalCommandBuffer : public CommandBuffer {
    public:
        virtual void bindVertex() = 0;
        virtual void bindPipeline() = 0;
        virtual void draw() = 0;
    }; // class GraphicalCommandBuffer
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_GRAPHICALCOMMANDBUFFER_H