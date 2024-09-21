#include "zeroengine_core/WindowSupport.hpp"

namespace ZEROengine {
    WindowSupport::WindowSupport(
        const std::string &title, 
        const WindowTransform &transform, 
        const WindowSetting &setting, 
        const WindowStyle &style
    ) :
    window_transform(transform),
    window_setting(setting),
    window_style(style),
    is_active(true),
    is_hidden(false),
    is_closing(false),
    closed(false),
    window_title(title)
    {}

    WindowSupport::~WindowSupport() {
        // None
    }

    void WindowSupport::resize(const uint32_t &new_frame_width, const uint32_t &new_frame_height) {
        this->window_transform.width = new_frame_width;
        this->window_transform.height = new_frame_height;
    }

    void WindowSupport::setTitle(const std::string &title) {
        this->window_title = title;
    }

    std::string WindowSupport::getTitle() const {
        return this->window_title;
    }

    bool WindowSupport::isMinimized() const {
        return this->window_setting.minimized;
    }

    void WindowSupport::minimize(const bool &status) {
        this->window_setting.minimized = status;
    }

    uint32_t WindowSupport::getWidth() const {
        return this->window_transform.width;
    }

    uint32_t WindowSupport::getHeight() const {
        return this->window_transform.height;
    }
    
    void WindowSupport::getDimensions(uint32_t &width_ret, uint32_t &height_ret) const {
        width_ret = this->window_transform.width;
        height_ret = this->window_transform.height;
    }

    uint32_t WindowSupport::getPositionX() const {
        return this->window_transform.x;
    }

    uint32_t WindowSupport::getPositionY() const {
        return this->window_transform.y;
    }

    bool WindowSupport::isFullscreen() const {
        return this->window_setting.fullscreen;
    }

    void WindowSupport::requestFullscreen() {
        this->window_setting.fullscreen = true;
    }

    bool WindowSupport::isActive() const {
        return this->is_active;
    }

    void WindowSupport::setActive(const bool &status) {
        this->is_active = status;
    }

    bool WindowSupport::isHidden() const {
        return this->is_hidden;
    }

    void WindowSupport::setHidden(const bool &status) {
        this->is_hidden = status;
    }

    void WindowSupport::reposition(const uint32_t &new_x, const uint32_t &new_y) {
        this->window_transform.x = new_x;
        this->window_transform.y = new_y;
    }

    bool WindowSupport::isClosing() const {
        return this->is_closing;
    }

    void WindowSupport::notifyClosing() {
        this->is_closing = true;
    }

    bool WindowSupport::isClosed() const {
        return this->closed;
    }
    
} // namespace ZEROengine