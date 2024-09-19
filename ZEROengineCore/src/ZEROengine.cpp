#include "zeroengine_core/ZEROengine.hpp"

#include <vector>
#include <array>
#include <iostream>

namespace ZEROengine {
    void ZEROengine::init() {
        this->graphical_module->initGraphicalModule();
    }

    void ZEROengine::run() {
        mainLoop();
    }

    void ZEROengine::mainLoop() {
        WindowContext *window_ctx;
        this->getGraphicalModule()->getWindowContext(&window_ctx);
        while(!this->quitting_signal) {
            window_ctx->poll();
            this->getGraphicalModule()->drawFrame();
            this->quitting_signal = window_ctx->isClosing();
        }
    }

    void ZEROengine::cleanup() {
        this->graphical_module->cleanup();
    }

    ZEROengine::~ZEROengine() {
        cleanup();
    }
} // namespace ZEROengine