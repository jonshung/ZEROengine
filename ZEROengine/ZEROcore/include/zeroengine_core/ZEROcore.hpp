#ifndef ZEROENGINE_CORE_H
#define ZEROENGINE_CORE_H

#include <memory>

#include "zeroengine_graphical/GPUModule.hpp"

namespace ZEROengine {
    class ZEROcore {
    private:
        std::shared_ptr<GPUModule> m_graphical_module;

    public:
        std::shared_ptr<GPUModule> getGraphicalModule();

    public:
        void cleanup();
    };
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_CORE_H