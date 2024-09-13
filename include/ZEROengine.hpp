#ifndef ZEROENGINE_H
#define ZEROENGINE_H

#include "VulkanGraphicalModule.hpp"
#include "VulkanPipelineBuffer.hpp"
#include "project_env.h"

#include <cstdint>
#include <fstream>
#include <vector>
#include <memory>
#include <utility>

#include <SDL3/SDL.h>

/**
 * @brief Engine runtime.
 */
class ZEROengine {
private:
    SDL_Event context_event;
    bool quitting_signal = false;
    VulkanGraphicalModule graphical_module;

public:
    void run();
    ~ZEROengine();

private:
    VkPipeline testing_pipeline = VK_NULL_HANDLE;

protected:
    /**
     * @brief A simple utitlity to load GPU program in the form of SPIR-V.
     * 
     * @param filename Source file path.
     * @return std::pair<char*, std::size_t>. A pointer to the source blob and its size.
     */
    static std::pair<char*, std::size_t> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }
        std::size_t f_size = (std::size_t) file.tellg();
        char* buffer = new char[f_size]; // DANGEROUS, i.e. please handle in the future - jonshung
        if(buffer == nullptr) {
            throw std::runtime_error("readFile() failed, err: cannot allocate space to accommodate file read");
        }
        file.seekg(0);
        file.read(buffer, f_size);
        file.close();
        return std::make_pair(buffer, f_size);
    }
private:
    /**
     * @brief Main engine loop, performing all game logic tasks and synchronization with other modules.
     */
    void mainLoop();
    void cleanup();

public:
    void quit() {
        this->quitting_signal = true;
    }
};  // Class ZEROengine

#endif // #ifndef ZEROENGINE_H