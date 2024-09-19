#ifndef ZEROENGINE_RENDER_WINDOW_H
#define ZEROENGINE_RENDER_WINDOW_H

#include "zeroengine_core/RenderTarget.hpp"

namespace ZEROengine {
    class RenderWindow : public RenderTarget {
        // None
    public:
        virtual void handleResize(const uint32_t &new_frame_width, const uint32_t &new_frame_height) = 0;
    }; // class RenderWindow
} // namespace ZEROengine
#endif // #ifndef ZEROENGINE_RENDER_WINDOW_H