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
        while(!this->quitting_signal) {
            if(this->getGraphicalModule()->isOff()) {
                break;
            }
            this->getGraphicalModule()->drawFrame();
        }
    }

    void ZEROengine::cleanup() {
        this->graphical_module->cleanup();
    }

    ZEROengine::~ZEROengine() {
        cleanup();
    }
} // namespace ZEROengine