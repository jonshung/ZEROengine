#ifndef ZEROENGINE_GRAPHICAL_MODULE_H
#define ZEROENGINE_GRAPHICAL_MODULE_H

#include "zeroengine_core/WindowContext.hpp"
#include "zeroengine_core/ZEROengineDefines.hpp"

#include <memory>
#include <stdexcept>

namespace ZEROengine {
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
        ZEROResult getWindowContext(WindowContext** ret) {
            *ret = this->window_context.get();
            return { ZERO_SUCCESS, "" };
        }

        virtual void initGraphicalModule() = 0;
        virtual void drawFrame() = 0;
        virtual void handleResize() = 0;
        
        virtual void cleanup() = 0;
    }; // class GraphicalModule
} // namespace ZEROengine

#endif //#ifndef ZEROENGINE_GRAPHICAL_MODULE_H