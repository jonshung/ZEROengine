#ifndef ZEROENGINE_GRAPHICAL_MODULE_H
#define ZEROENGINE_GRAPHICAL_MODULE_H

#include "zeroengine_core/WindowContext.hpp"

#include <memory>
#include <stdexcept>

class GraphicalModule {
protected:
    std::unique_ptr<WindowContext> window_context;

public:
    void bindWindowContext(WindowContext* window_ctx) {
        if(!window_ctx) {
            throw std::runtime_error("GraphicalModule::bindWindowContext() failed, error: null pointer");
        }
        this->window_context = std::unique_ptr<WindowContext>(window_ctx);
    }
    WindowContext* getWindowContext() {
        return this->window_context.get();
    }

    virtual void initGraphicalModule() = 0;
    virtual void drawFrame() = 0;
    virtual void handleResize() = 0;
    
    virtual void cleanup() = 0;
};

#endif //#ifndef ZEROENGINE_GRAPHICAL_MODULE_H