#include <cstdint>
#include <fstream>
#include <memory>
#include <utility>
#include <unordered_map>
#include <vector>
#include <array>
#include <iostream>

#include "zeroengine_core/ApplicationContext.hpp"
#include "zeroengine_graphical/GraphicalModule.hpp"

namespace ZEROengine {
    void ApplicationContext::init() {
    }

    void ApplicationContext::run() {
        mainLoop();
    }

    void ApplicationContext::mainLoop() {
        ZERO_ASSERT(this->m_core != nullptr, "Core is not initialized!");

        std::shared_ptr<GraphicalModule> graphical_module = this->m_core->getGraphicalModule();
        ZERO_ASSERT(graphical_module != nullptr, "Graphical module not found!");

        while(!this->m_quitting_signal) {
            if(graphical_module->isOff()) {
                break;
            }
            graphical_module->drawFrame();
        }
    }

    void ApplicationContext::cleanup() {
        this->m_core->cleanup();
    }

    ApplicationContext::~ApplicationContext() {
        this->cleanup();
    }
} // namespace ZEROengine