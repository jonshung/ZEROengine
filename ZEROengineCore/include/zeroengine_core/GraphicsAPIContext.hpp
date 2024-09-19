#ifndef ZEROENGINE_GRAPHICS_API_CONTEXT_H
#define ZEROENGINE_GRAPHICS_API_CONTEXT_H

#include "zeroengine_core/ZEROengineDefines.hpp"

namespace ZEROengine {
    class GraphicsAPIContext {
    public:
        virtual ZEROResult allocateBuffer() = 0;
        virtual ZEROResult allocateTexture() = 0;

        virtual void cleanup() = 0;
    }; // class GraphicsAPIContext
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_GRAPHICS_API_CONTEXT_H