#ifndef ZEROENGINE_GRAPHICALCONTEXT_H
#define ZEROENGINE_GRAPHICALCONTEXT_H

#include <memory>

namespace ZEROengine {
    class Context {
    public:
        virtual void init() = 0;
        virtual void cleanup() = 0;

        virtual void beginRecording() = 0;
        virtual void endRecording() = 0;
    };

    class GraphicalContext : public Context {
    public:
        virtual void bindVertex() = 0;
        virtual void bindPipeline() = 0;
        virtual void draw() = 0;
    };
}
#endif // #ifndef ZEROENGINE_GRAPHICALCONTEXT_H