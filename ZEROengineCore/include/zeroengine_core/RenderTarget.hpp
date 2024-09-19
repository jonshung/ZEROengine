#ifndef ZEROENGINE_RENDER_TARGET_H
#define ZEROENGINE_RENDER_TARGET_H

#include <cstdint>

namespace ZEROengine {
    class RenderTarget {
    protected:
        uint32_t target_width;
        uint32_t target_height;
        bool target_active = true;
    public:
        RenderTarget() {}
        virtual void getDimensions(uint32_t &ret_width, uint32_t &ret_height) const;
        virtual void setDimensions(const uint32_t &new_width, const uint32_t &new_height);
        virtual bool isActive() const;
        virtual void setActive(const bool &status);
    }; // class RenderTarget
} // namespace ZEROengine
#endif // #ifndef ZEROENGINE_RENDER_TARGET_H