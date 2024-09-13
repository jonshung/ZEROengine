#include "SDLWindowContext.hpp"

#include <stdexcept>

void SDLWindowContext::initWindow(uint32_t width, uint32_t height) {
    if(!this->window_handler && (this->window_handler = SDL_CreateWindow("ZERO/engine", width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE)) == nullptr) {
        throw std::runtime_error("SDL_CreateWindow() failed, err: " + std::string(SDL_GetError()));
    }
}

const char*const* SDLWindowContext::requestInstanceExtensions(uint32_t &ext_count) const {
    return SDL_Vulkan_GetInstanceExtensions(&ext_count);
}

void SDLWindowContext::getSurface(const VkInstance& vk_instance, VkSurfaceKHR* dest) {
    if(this->surface_handler) {
        dest = this->surface_handler;
        return;
    }
    if(SDL_Vulkan_CreateSurface(this->window_handler, vk_instance, nullptr, dest) < 0) {
        throw std::runtime_error("SDLWindowContext::getSurface() failed, err: " + std::string(SDL_GetError()));
    }
    this->surface_handler = dest;
}

void SDLWindowContext::getFramebufferSize(int &width, int &height) {
    SDL_GetWindowSizeInPixels(this->window_handler, &width, &height);
}

bool SDLWindowContext::isMinimized() {
    return (SDL_GetWindowFlags(this->window_handler) & SDL_WINDOW_MINIMIZED);
}

void SDLWindowContext::cleanup() {
    SDL_DestroyWindow(this->window_handler);
}