#ifndef ZEROENGINE_RENDERER_H
#define ZEROENGINE_RENDERER_H

#include "zeroengine_core/ZEROengineDefines.hpp"

namespace ZEROengine {
    class Renderer {
    public:
        virtual ZEROResult initRenderer() = 0;
    }; // class Renderer
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_RENDERER_H