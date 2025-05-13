#ifndef ZEROENGINE_VULKAN_XCB_WINDOW_H
#define ZEROENGINE_VULKAN_XCB_WINDOW_H

#include <cstdint>
#include <string>

#include "zeroengine_vulkan/VulkanDefines.hpp"
#include "zeroengine_vulkan/VulkanWindow.hpp"

#include <xcb/xcb.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xcb.h>

namespace ZEROengine {
    class VulkanXCBWindow final : public VulkanWindow {
        xcb_connection_t* m_xcb_connection;
        xcb_screen_t* m_xcb_screen;
        unsigned m_xcb_windowid;

        xcb_atom_t m_xcb_wm_protocols;
        xcb_atom_t m_xcb_wm_delete_window;
        xcb_atom_t m_xcb_wm_net_state;
        xcb_atom_t m_xcb_wm_fullscreen;

    private:
        void xcbConnect();
        void createWindow(
            const std::string &name, 
            const uint32_t &width, 
            const uint32_t &height, 
            const std::unordered_map<std::string, std::string> &options,
            const bool &fullscreen
        );
        void closeWindow();

    public:
        VulkanXCBWindow(
            VulkanContext *vulkan_context,
            const std::string &title,
            const WindowTransform &transform,
            const WindowSetting &setting,
            const WindowStyle &style
        );
        ~VulkanXCBWindow() override;

        void init(void *param) override;

        void pollEvent() override;
        void setHidden(const bool &status) override;

        void cleanup() override;

        void handleMoveOrResize() override;
    };
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKAN_XCB_WINDOW_H