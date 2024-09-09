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
    void run() {
        if(SDL_WasInit(SDL_INIT_VIDEO) == 0 && SDL_Init(SDL_INIT_VIDEO) < 0) {
            throw std::runtime_error("SDL_Init() failed, err: " + std::string(SDL_GetError()));
        }
        window_context.initWindow(640, 480);
        vulkan_context.initVulkan(&window_context);
        auto frag_data = readFile(std::string(PROJECT_BINARY_TEST_DIR) + "/frag.spv");
        auto vert_data = readFile(std::string(PROJECT_BINARY_TEST_DIR) + "/vert.spv");
        
        vulkan_context.getScreenRenderer().createGraphicsPipelines(vulkan_context.getDevice(), { {vert_data, frag_data} });
        mainLoop();
    }

    ~BasicApp();

private:
    SDL_Event context_event;
    bool quitting_signal = false;

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