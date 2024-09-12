#ifndef SDLWINDOW_CONTEXT_H
#define SDLWINDOW_CONTEXT_H

#include <SDL3/SDL_video.h>
#include "WindowContext.hpp"
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL.h>

#include <cstdint>

class SDLWindowContext : public WindowContext {
private:
    SDL_Window *window_handler = nullptr;
    VkSurfaceKHR *surface_handler = nullptr;

public:
    void initWindow(uint32_t width, uint32_t height) override;
    const char*const* requestInstanceExtensions(uint32_t &ext_count) const override;
    void getSurface(const VkInstance& vk_instance, VkSurfaceKHR* vk_surface) override;
    void getFramebufferSize(int &width, int &height) override;
    bool isMinimized() override;
    
    virtual void cleanup() override;

private:
};
#endif // #ifndef SDLWINDOW_CONTEXT_H