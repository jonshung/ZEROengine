#ifndef ZEROENGINE_APPLICATIONCONTEXT_H
#define ZEROENGINE_APPLICATIONCONTEXT_H

#include "zeroengine_core/ZEROcore.hpp"
#include <memory>

namespace ZEROengine {
    class ApplicationContext {
    private:
        bool m_quitting_signal = false;

    protected:
        std::shared_ptr<ZEROcore> m_core;

    public:
        void init();
        void run();

    private:
        /**
         * @brief Main engine loop, performing all game logic tasks and synchronization with other modules.
         */
        void mainLoop();
        void cleanup();

    public:
        void quit();
        ~ApplicationContext();
    };  // class ApplicationContext
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_APPLICATIONCONTEXT_H