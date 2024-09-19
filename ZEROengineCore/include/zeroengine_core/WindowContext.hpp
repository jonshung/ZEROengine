#ifndef ZEROENGINE_WINDOW_CONTEXT_H
#define ZEROENGINE_WINDOW_CONTEXT_H

#include <cstdint>

namespace ZEROengine {
    /**
     * @brief General purpose WindowContext to manage the application window.
     * 
     */
    class WindowContext {
    public:
        /**
         * @brief Initialization function. Any uses to this class of objects (including it's child class) 
         * MUST call this function first before any window operation.
         * 
         * @param width 
         * @param height 
         */
        virtual void initWindow(uint32_t width, uint32_t height) = 0;

        /**
         * @brief Get the Framebuffer size.
         * 
         * @param[out] width 
         * @param[out] height 
         */
        virtual void getFramebufferSize(uint32_t &width, uint32_t &height) = 0;
        virtual void poll() = 0;
        virtual bool isMinimized() = 0;
        virtual bool resized() = 0;
        virtual bool isClosing() = 0;

        virtual void cleanup() = 0;
    }; // class WindowContext
} // namespace ZEROengine

#endif // #ifndef WINDOW_CONTEXT_ABSTRACT_H
