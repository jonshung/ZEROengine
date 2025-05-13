#include "zeroengine_graphical/GraphicalWindow.hpp"

namespace ZEROengine {
    GraphicalWindow::GraphicalWindow(
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

    GraphicalWindow::~GraphicalWindow() {
        // None
    }

    void GraphicalWindow::resize(const uint32_t &new_frame_width, const uint32_t &new_frame_height) {
        this->m_window_transform.width = new_frame_width;
        this->m_window_transform.height = new_frame_height;
    }

    void GraphicalWindow::setTitle(const std::string &title) {
        this->window_title = title;
    }

    std::string GraphicalWindow::getTitle() const {
        return this->window_title;
    }

    bool GraphicalWindow::isMinimized() const {
        return this->m_window_setting.minimized;
    }

    void GraphicalWindow::minimize(const bool &status) {
        this->m_window_setting.minimized = status;
    }

    uint32_t GraphicalWindow::getWidth() const {
        return this->m_window_transform.width;
    }

    uint32_t GraphicalWindow::getHeight() const {
        return this->m_window_transform.height;
    }
    
    void GraphicalWindow::getDimensions(uint32_t &width_ret, uint32_t &height_ret) const {
        width_ret = this->m_window_transform.width;
        height_ret = this->m_window_transform.height;
    }

    uint32_t GraphicalWindow::getPositionX() const {
        return this->m_window_transform.x;
    }

    uint32_t GraphicalWindow::getPositionY() const {
        return this->m_window_transform.y;
    }

    bool GraphicalWindow::isFullscreen() const {
        return this->m_window_setting.fullscreen;
    }

    void GraphicalWindow::requestFullscreen() {
        this->m_window_setting.fullscreen = true;
    }

    bool GraphicalWindow::isActive() const {
        return this->m_is_active;
    }

    void GraphicalWindow::setActive(const bool &status) {
        this->m_is_active = status;
    }

    bool GraphicalWindow::isHidden() const {
        return this->m_is_hidden;
    }

    void GraphicalWindow::setHidden(const bool &status) {
        this->m_is_hidden = status;
    }

    void GraphicalWindow::reposition(const uint32_t &new_x, const uint32_t &new_y) {
        this->m_window_transform.x = new_x;
        this->m_window_transform.y = new_y;
    }

    bool GraphicalWindow::isClosing() const {
        return this->m_is_closing;
    }

    void GraphicalWindow::notifyClosing() {
        this->m_is_closing = true;
    }

    bool GraphicalWindow::isClosed() const {
        return this->m_is_closed;
    }
    
} // namespace ZEROengine