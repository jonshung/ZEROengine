#include <memory>

#include "zeroengine_core/ZEROcore.hpp"

namespace ZEROengine {
    std::shared_ptr<GraphicalModule> ZEROcore::getGraphicalModule() {
        return this->m_graphical_module;
    }
}