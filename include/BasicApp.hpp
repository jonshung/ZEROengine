#ifndef BASIC_APP_H
#define BASIC_APP_H

#include "SDLWindowContext.hpp"
#include "VulkanContext.hpp"
#include "project_env.h"
#include "VulkanRenderer_def.hpp"

#include <cstdint>
#include <tuple>
#include <fstream>

#include <SDL3/SDL.h>

class BasicApp {
    // Renderer should be here
private:
    SDLWindowContext window_context;
    VulkanContext vulkan_context;

public:
    void run();
    ~BasicApp();

private:
    SDL_Event context_event;
    bool quitting_signal = false;
    VkPipeline testing_pipeline = VK_NULL_HANDLE;

protected:
    static std::tuple<char*, size_t> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }
        size_t f_size = (size_t) file.tellg();
        char* buffer = new char[f_size]; // DANGEROUS, i.e. please handle in the future - jonshung
        if(buffer == nullptr) {
            throw std::runtime_error("readFile() failed, err: cannot allocate space to accommodate file read");
        }
        file.seekg(0);
        file.read(buffer, f_size);
        file.close();
        return std::make_tuple(buffer, f_size);
    }
private:
    void mainLoop();

    void drawFrame();
    void cleanup();
};  // BasicApp

#endif // #ifndef BASIC_APP_H