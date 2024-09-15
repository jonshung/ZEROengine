#ifndef ZEROENGINE_SDLWINDOW_CONTEXT_H
#define ZEROENGINE_SDLWINDOW_CONTEXT_H

#include "zeroengine_vulkan/VulkanWindowContext.hpp"

#include <SDL3/SDL_video.h>
#include <SDL3/SDL_vulkan.h>
#include <SDL3/SDL.h>

#include <cstdint>

/**
 * @brief The basic SDL implementation for window and surface management 
 * 
 */
class SDL_VulkanWindowContext : public VulkanWindowContext {
private:
    SDL_Window *window_handler = nullptr;
    SDL_Event window_event;

public:
    void initWindow(uint32_t width, uint32_t height) override;
    void getSurface(const VkInstance& vk_instance, VkSurfaceKHR* vk_surface) override;
    void getFramebufferSize(uint32_t &width, uint32_t &height) override;
    void poll() override;
    bool isMinimized() override;
    bool isClosing() override;
    
    virtual void cleanup() override;

private:
};
#endif // #ifndef SDLWINDOW_CONTEXT_H