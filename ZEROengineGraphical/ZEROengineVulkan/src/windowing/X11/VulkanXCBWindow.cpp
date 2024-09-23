#include "zeroengine_vulkan/windowing/X11/VulkanXCBWindow.hpp"

#include <string>

namespace ZEROengine {
    static xcb_intern_atom_cookie_t zero_xcb_intern_atom_cookie(xcb_connection_t *connection, const std::string &name) {
        return xcb_intern_atom(connection, 0, static_cast<uint16_t>(name.size()), name.c_str());
    }

    static xcb_atom_t zero_xcb_intern_atom(xcb_connection_t *connection, xcb_intern_atom_cookie_t cookie) {
        xcb_atom_t atom = XCB_ATOM_NONE;
        xcb_intern_atom_reply_t *reply = xcb_intern_atom_reply(connection, cookie, 0 );
        if(reply){
            atom = reply->atom;
            free(reply);
        }
        return atom;
    }

    VulkanXCBWindow::VulkanXCBWindow(
        VulkanContext *vulkan_context,
        const std::string &title,
        const WindowTransform &transform,
        const WindowSetting &setting,
        const WindowStyle &style
    ) :
    VulkanWindow(vulkan_context, title, transform, setting, style),
    xcb_connection(nullptr),
    xcb_screen(nullptr),
    xcb_windowid(0),
    xcb_wm_protocols(0),
    xcb_wm_delete_window(0),
    xcb_wm_net_state(0),
    xcb_wm_fullscreen(0)
    {}

    VulkanXCBWindow::~VulkanXCBWindow() {
        VulkanWindow::cleanup();
    }

    void VulkanXCBWindow::xcbConnect() {
        int ret_scr = 0;
        this->xcb_connection = xcb_connect(nullptr, &ret_scr); // always ret non-null
        if(xcb_connection_has_error(this->xcb_connection)) {
            xcb_disconnect(this->xcb_connection);
            ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "Failed to init XCB connection");
        }
        xcb_screen_iterator_t xcb_scr_iter = xcb_setup_roots_iterator(xcb_get_setup(this->xcb_connection));
        while(xcb_scr_iter.rem && ret_scr > 0) {
            --ret_scr;
            xcb_screen_next(&xcb_scr_iter);
        }
        this->xcb_screen = xcb_scr_iter.data;
    }

    void VulkanXCBWindow::createWindow(
        const std::string &name, 
        const uint32_t &width, 
        const uint32_t &height, 
        const std::unordered_map<std::string, std::string> &options,
        const bool &fullscreen
    ) {
        (void)options;
        this->xcb_windowid = xcb_generate_id(this->xcb_connection);
        
        uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK,
        value_list[2] = {
            this->xcb_screen->black_pixel,
            XCB_EVENT_MASK_STRUCTURE_NOTIFY // might need more in the future.
        };

        xcb_create_window(
            this->xcb_connection, 
            XCB_COPY_FROM_PARENT, 
            this->xcb_windowid,
            this->xcb_screen->root,
            static_cast<uint16_t>(this->getPositionX()),
            static_cast<uint16_t>(this->getPositionY()),
            static_cast<uint16_t>(width),
            static_cast<uint16_t>(height),
            0, // border width, not supported yet
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            this->xcb_screen->root_visual,
            mask, value_list
        );

        // setting title
        xcb_intern_atom_cookie_t utf8_type_cookie = zero_xcb_intern_atom_cookie(this->xcb_connection, "UTF8_STRING");
        xcb_intern_atom_cookie_t window_title_cookie = zero_xcb_intern_atom_cookie(this->xcb_connection, "WM_NAME");
        xcb_atom_t utf8_type = zero_xcb_intern_atom(this->xcb_connection, utf8_type_cookie );
        xcb_atom_t window_title = zero_xcb_intern_atom(this->xcb_connection, window_title_cookie);
        xcb_change_property(this->xcb_connection, XCB_PROP_MODE_REPLACE, this->xcb_windowid, 
                        window_title, utf8_type, 8u, name.size(), name.c_str());

        xcb_intern_atom_cookie_t wm_protocols_cookie = zero_xcb_intern_atom_cookie(this->xcb_connection, "WM_PROTOCOLS");
        xcb_intern_atom_cookie_t wm_delete_cookie = zero_xcb_intern_atom_cookie(this->xcb_connection, "WM_DELETE_WINDOW");
        this->xcb_wm_protocols = zero_xcb_intern_atom(this->xcb_connection, wm_protocols_cookie);
        this->xcb_wm_delete_window = zero_xcb_intern_atom(this->xcb_connection, wm_delete_cookie);
        xcb_change_property(this->xcb_connection, XCB_PROP_MODE_REPLACE, this->xcb_windowid, xcb_wm_protocols, XCB_ATOM_ATOM,
                             32u, 1u, &this->xcb_wm_delete_window);

        xcb_intern_atom_cookie_t wm_net_state_cookie = zero_xcb_intern_atom_cookie(this->xcb_connection, "_NET_WM_STATE");
        xcb_intern_atom_cookie_t wm_fullscreen_cookie = zero_xcb_intern_atom_cookie(this->xcb_connection, "_NET_WM_STATE_FULLSCREEN");
        this->xcb_wm_net_state = zero_xcb_intern_atom(this->xcb_connection, wm_net_state_cookie);
        this->xcb_wm_fullscreen = zero_xcb_intern_atom(this->xcb_connection, wm_fullscreen_cookie);
        if(this->xcb_wm_net_state != XCB_ATOM_NONE && this->xcb_wm_fullscreen != XCB_ATOM_NONE && fullscreen) {
            xcb_change_property(this->xcb_connection, XCB_PROP_MODE_REPLACE, this->xcb_windowid, this->xcb_wm_net_state,
                                 XCB_ATOM_ATOM, 32u, 1u, &xcb_wm_fullscreen);
            WindowSupport::requestFullscreen();
        }
        xcb_flush(this->xcb_connection);
    }

    void VulkanXCBWindow::pollEvent() {
        if(closed || !this->xcb_connection) {
            return;
        }
        xcb_generic_event_t *event_data = nullptr;
        while( (event_data = xcb_poll_for_event(this->xcb_connection)) ) {
            switch( (event_data->response_type & ~0x80) ) {
            case XCB_CLIENT_MESSAGE: {
                xcb_client_message_event_t *event_detail = reinterpret_cast<xcb_client_message_event_t *>(event_data);
                if( event_detail->format == 32u && 
                    event_detail->type == this->xcb_wm_protocols &&
                    event_detail->data.data32[0] == this->xcb_wm_delete_window
                ) {
                    this->notifyClosing();
                    break;
                }
            }
            }
            free(event_data);
            if(this->isClosing()) break;
        }
    }

    void VulkanXCBWindow::closeWindow() {
        this->cleanup();
    }

    void VulkanXCBWindow::cleanup() {
        if(closed){
            return;
        }
        
        is_closing = false;
        closed = true;
        this->setActive(false);

        xcb_destroy_window(this->xcb_connection, this->xcb_windowid);
        xcb_flush(this->xcb_connection);
        xcb_disconnect(this->xcb_connection);
        VulkanWindow::cleanup();
    }

    void VulkanXCBWindow::init(void *param) {
        (void)param;
        // native windowing
        if(!this->xcb_connection || !this->xcb_screen) {
            this->xcbConnect();
        }
        if(!this->xcb_windowid) {
            this->createWindow(this->getTitle(), this->getWidth(), this->getHeight(), {}, this->isFullscreen());
        }
        this->setHidden(false);
        
        // VulkanContext's instance and physical device is guaranteed to exists at this moment
        // The windowing system's job is to query the physical device for at least a queue family
        // that supports Graphics and a family that supports Presentation.
        // Physical device selection already guarantees a Graphics queue, now we only need to query for Presentation
        uint32_t families_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(this->vulkan_context->getPhysicalDevice(), &families_count, nullptr);
        VkBool32 found_presentation = VK_FALSE;

        for(std::size_t q_index = 0; q_index < families_count; ++q_index) {
            if( (found_presentation = vkGetPhysicalDeviceXcbPresentationSupportKHR(
                this->vulkan_context->getPhysicalDevice(), 
                q_index,
                this->xcb_connection,
                this->xcb_screen->root_visual
            )) == VK_TRUE ) break;
        }
        if(found_presentation == VK_FALSE) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "Vulkan cannot support creating surface on current X11 screen with current selected device");
        }

        VkXcbSurfaceCreateInfoKHR surface_create_info{};
        surface_create_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
        surface_create_info.connection = this->xcb_connection;
        surface_create_info.window = this->xcb_windowid;
        ZERO_VK_CHECK_EXCEPT(vkCreateXcbSurfaceKHR(this->vulkan_context->getInstance(), &surface_create_info, nullptr, &this->vk_surface));
        
        // must call this initialize logical devices
        this->vulkan_context->VKInit_initLogicalDevice(this->vk_surface);

        initSwapChain();
        initSwapChainRenderPass();
        initSwapChainRenderTargets();
    }

    void VulkanXCBWindow::handleMoveOrResize() {
        if(isClosed() || !this->xcb_windowid) {
            return;
        }

        xcb_get_geometry_cookie_t geom_cookie = xcb_get_geometry(this->xcb_connection, this->xcb_windowid);
        xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(this->xcb_connection, geom_cookie, NULL );

        if(!isFullscreen())
        {
            this->window_transform.x = geom->x;
            this->window_transform.y = geom->y;
        }

        this->resize(geom->width, geom->height);
        this->vulkan_context->stall();
        reload_swapChain();
        free(geom);
    }

    void VulkanXCBWindow::setHidden(const bool &status) {
        WindowSupport::setHidden(status);

        if(!status) {
            xcb_map_window(this->xcb_connection, this->xcb_windowid);
        } else {
            xcb_unmap_window(this->xcb_connection, this->xcb_windowid);
        }
        xcb_flush(this->xcb_connection);
    }
} // namespace ZEROengine
        