#ifndef ZEROENGINE_GRAPHICALWINDOW_H
#define ZEROENGINE_GRAPHICALWINDOW_H

#include "zeroengine_core/ZERODefines.hpp"

#include <string>
#include <cstdint>

namespace ZEROengine {
    struct WindowTransform {
        uint32_t x; // display x positioning (top left anchor)
        uint32_t y; // display y position (top left anchor)
        
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t min_width = 0;
        uint32_t min_height = 0;
        uint32_t max_width = 0;
        uint32_t max_height = 0;
    };

    struct WindowSetting {
        bool fullscreen = false;
        bool minimized = false;

        bool can_fullscreen = false;
        bool can_minimize = true;
        bool resizable = true;
        bool movable = true;
    };

    struct WindowStyle {
        uint32_t bg_color;
        bool transparent = false;
    };

    class GraphicalWindow {
    protected:
        WindowTransform m_window_transform;
        WindowSetting m_window_setting;
        WindowStyle m_window_style;

        bool m_is_active;
        bool m_is_hidden;
        bool m_is_closing;
        bool m_is_closed;

        std::string window_title;

    public:
        GraphicalWindow(const std::string &title, const WindowTransform &transform, const WindowSetting &setting, const WindowStyle &style);
        virtual ~GraphicalWindow();

        virtual void init(void *params) = 0;

        virtual void setTitle(const std::string &title);
        virtual std::string getTitle() const;

        virtual bool isMinimized() const;
        virtual void minimize(const bool &status);

        virtual uint32_t getWidth() const;
        virtual uint32_t getHeight() const;
        virtual void getDimensions(uint32_t &width_ret, uint32_t &height_ret) const;

        virtual uint32_t getPositionX() const;
        virtual uint32_t getPositionY() const;

        virtual bool isFullscreen() const;
        virtual void requestFullscreen();

        virtual bool isActive() const;
        virtual void setActive(const bool &status);

        virtual bool isHidden() const;
        virtual void setHidden(const bool &status);

        virtual void reposition(const uint32_t &new_x, const uint32_t &new_y);

        virtual void resize(const uint32_t &new_frame_width, const uint32_t &new_frame_height);
        virtual void handleMoveOrResize() = 0;

        virtual void pollEvent() = 0;
        virtual bool isClosing() const;
        virtual bool isClosed() const;
        virtual void notifyClosing();

        virtual void cleanup() {};
    }; // class GraphicalWindow
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_GRAPHICALWINDOW_H