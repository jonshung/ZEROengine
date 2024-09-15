#ifndef ZEROENGINE_H
#define ZEROENGINE_H

#include "zeroengine_core/GraphicalModule.hpp"

#include <cstdint>
#include <fstream>
#include <vector>
#include <memory>
#include <utility>

class ZEROengine {
private:
    bool quitting_signal = false;
    std::unique_ptr<GraphicalModule> graphical_module;

public:
    void init();
    void run();
    void bindGraphicalModule(GraphicalModule* module) {
        this->graphical_module = std::unique_ptr<GraphicalModule>(module);
    }
    GraphicalModule* getGraphicalModule() {
        return this->graphical_module.get();
    }
    ~ZEROengine();

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