#ifndef ZEROENGINE_GPUMODULE_H
#define ZEROENGINE_GPUMODULE_H

#include "zeroengine_core/ZERODefines.hpp"

#include <memory>
#include <stdexcept>
#include <memory>

namespace ZEROengine {
    class GPUModule {
    protected:
        bool m_is_off = false;
    public:
        GPUModule() {}

        virtual void initGraphicalModule() = 0;

        virtual void drawFrame() = 0;
        virtual bool isOff() const final { return m_is_off; }
        
        virtual void cleanup() = 0;
    }; // class GPUModule
} // namespace ZEROengine

#endif //#ifndef ZEROENGINE_GPUMODULE_H