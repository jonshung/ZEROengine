#include "zeroengine_graphical/GPUWindow.hpp"

namespace ZEROengine {
    GPUWindow::GPUWindow(
        const std::string &title, 
        const WindowTransform &transform, 
        const WindowSetting &setting, 
        const WindowStyle &style
    ) :
    m_window_transform(transform),
    m_window_setting(setting),
    m_window_style(style),
    m_is_active(true),
    m_is_hidden(false),
    m_is_closing(false),
    m_is_closed(false),
    window_title(title)
    {}

    GPUWindow::~GPUWindow() {
        // None
    }

    void GPUWindow::resize(const uint32_t &new_frame_width, const uint32_t &new_frame_height) {
        m_window_transform.width = new_frame_width;
        m_window_transform.height = new_frame_height;
    }

    void GPUWindow::setTitle(const std::string &title) {
        window_title = title;
    }

    std::string GPUWindow::getTitle() const {
        return window_title;
    }

    bool GPUWindow::isMinimized() const {
        return m_window_setting.minimized;
    }

    void GPUWindow::minimize(const bool &status) {
        m_window_setting.minimized = status;
    }

    uint32_t GPUWindow::getWidth() const {
        return m_window_transform.width;
    }

    uint32_t GPUWindow::getHeight() const {
        return m_window_transform.height;
    }
    
    void GPUWindow::getDimensions(uint32_t &width_ret, uint32_t &height_ret) const {
        width_ret = m_window_transform.width;
        height_ret = m_window_transform.height;
    }

    uint32_t GPUWindow::getPositionX() const {
        return m_window_transform.x;
    }

    uint32_t GPUWindow::getPositionY() const {
        return m_window_transform.y;
    }

    bool GPUWindow::isFullscreen() const {
        return m_window_setting.fullscreen;
    }

    void GPUWindow::requestFullscreen() {
        m_window_setting.fullscreen = true;
    }

    bool GPUWindow::isActive() const {
        return m_is_active;
    }

    void GPUWindow::setActive(const bool &status) {
        m_is_active = status;
    }

    bool GPUWindow::isHidden() const {
        return m_is_hidden;
    }

    void GPUWindow::setHidden(const bool &status) {
        m_is_hidden = status;
    }

    void GPUWindow::reposition(const uint32_t &new_x, const uint32_t &new_y) {
        m_window_transform.x = new_x;
        m_window_transform.y = new_y;
    }

    bool GPUWindow::isClosing() const {
        return m_is_closing;
    }

    void GPUWindow::notifyClosing() {
        m_is_closing = true;
    }

    bool GPUWindow::isClosed() const {
        return m_is_closed;
    }
} // namespace ZEROengine