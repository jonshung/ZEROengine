#include "zeroengine_sdl/SDL_VulkanWindowContext.hpp"

#include <stdexcept>

void SDL_VulkanWindowContext::initWindow(uint32_t width, uint32_t height) {
    if(SDL_WasInit(SDL_INIT_VIDEO) == 0 && SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw std::runtime_error("SDL_Init() failed, err: " + std::string(SDL_GetError()));
    }
    if(!this->window_handler && (this->window_handler = SDL_CreateWindow("ZERO/engine", width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE)) == nullptr) {
        throw std::runtime_error("SDL_CreateWindow() failed, err: " + std::string(SDL_GetError()));
    }
}

void SDL_VulkanWindowContext::poll() {
    SDL_PollEvent(&this->window_event);
}

void SDL_VulkanWindowContext::getSurface(const VkInstance& vk_instance, VkSurfaceKHR* dest) {
    if(this->surface_handler) {
        dest = this->surface_handler;
        return;
    }
    if(SDL_Vulkan_CreateSurface(this->window_handler, vk_instance, nullptr, dest) < 0) {
        throw std::runtime_error("SDLWindowContext::getSurface() failed, err: " + std::string(SDL_GetError()));
    }
    this->surface_handler = dest;
}

void SDL_VulkanWindowContext::getFramebufferSize(uint32_t &width, uint32_t &height) {
    int w, h;
    SDL_GetWindowSizeInPixels(this->window_handler, &w, &h);
    width = w; height = h;
}

bool SDL_VulkanWindowContext::isMinimized() {
    return (SDL_GetWindowFlags(this->window_handler) & SDL_WINDOW_MINIMIZED);
}

bool SDL_VulkanWindowContext::isClosing() {
    return (this->window_event.type == SDL_EVENT_QUIT);
}

void SDL_VulkanWindowContext::cleanup() {
    SDL_DestroyWindow(this->window_handler);
    SDL_Quit();
}