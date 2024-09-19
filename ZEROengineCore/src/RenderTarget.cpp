#include "zeroengine_core/RenderTarget.hpp"

namespace ZEROengine {
    bool RenderTarget::isActive() const {
        return this->target_active;
    }
    void RenderTarget::setActive(const bool &status) {
        this->target_active = status;
    }
    void RenderTarget::getDimensions(uint32_t &ret_width, uint32_t &ret_height) const {
        ret_width = this->target_width;
        ret_height = this->target_height;
    }
    void RenderTarget::setDimensions(const uint32_t &new_width, const uint32_t &new_height) {
        this->target_width = new_width;
        this->target_height = new_height;
    }
} // namespace ZEROengine