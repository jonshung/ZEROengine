#include <memory>

#include "zeroengine_core/ApplicationContext.hpp"
#include "zeroengine_graphical/GraphicalModule.hpp"

namespace ZEROengine {
    void ApplicationContext::init() {
    }

    void ApplicationContext::run() {
        mainLoop();
    }

    void ApplicationContext::mainLoop() {
        ZERO_ASSERT(m_core != nullptr, "Core is not initialized!");

        std::shared_ptr<GraphicalModule> graphical_module = m_core->getGraphicalModule();
        ZERO_ASSERT(graphical_module != nullptr, "Graphical module not found!");

        while(!m_quitting_signal) {
            if(graphical_module->isOff()) {
                break;
            }
            graphical_module->drawFrame();
        }
    }

    void ApplicationContext::quit() {
        m_quitting_signal = true;
    }

    void ApplicationContext::cleanup() {
        m_core->cleanup();
    }

    ApplicationContext::~ApplicationContext() {
        cleanup();
    }
} // namespace ZEROengine