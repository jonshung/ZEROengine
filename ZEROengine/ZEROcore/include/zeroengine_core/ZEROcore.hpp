#ifndef ZEROENGINE_CORE_H
#define ZEROENGINE_CORE_H

#include <memory>

#include "zeroengine_graphical/GraphicalModule.hpp"

namespace ZEROengine {
    class ZEROcore {
    private:
        std::shared_ptr<GraphicalModule> m_graphical_module;

    public:
        std::shared_ptr<GraphicalModule> getGraphicalModule();

    public:
        void cleanup();
    };
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_CORE_H