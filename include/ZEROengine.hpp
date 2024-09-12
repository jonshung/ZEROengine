#ifndef ZEROENGINE_H
#define ZEROENGINE_H

#include "VulkanGraphicalModule.hpp"
#include "project_env.h"

#include <cstdint>
#include <tuple>
#include <fstream>
#include <vector>
#include <memory>

#include <SDL3/SDL.h>

class ZEROengine {
private:
    SDL_Event context_event;
    std::shared_ptr<SDLWindowContext> window_context;
    bool quitting_signal = false;
    std::unique_ptr<VulkanGraphicalModule> graphical_module;
public:
    void run();
    ~ZEROengine();

private:
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
    void cleanup();

public:
    void quit() {
        this->quitting_signal = true;
    }
};  // Class ZEROengine

#endif // #ifndef ZEROENGINE_H