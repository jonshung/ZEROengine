#ifndef ZEROENGINE_GRAPHICALMODULE_H
#define ZEROENGINE_GRAPHICALMODULE_H

#include "zeroengine_core/ZERODefines.hpp"

#include <memory>
#include <stdexcept>
#include <memory>

namespace ZEROengine {
    class GraphicalModule {
    protected:
        bool m_is_off = false;
    public:
        GraphicalModule() {}

        virtual void initGraphicalModule() = 0;
        virtual void drawFrame() = 0;
        virtual bool isOff() const final { return m_is_off; }
        
        virtual void cleanup() = 0;
    }; // class GraphicalModule
} // namespace ZEROengine

#endif //#ifndef ZEROENGINE_GRAPHICALMODULE_H