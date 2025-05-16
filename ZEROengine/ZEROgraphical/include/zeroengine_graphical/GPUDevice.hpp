#ifndef ZEROENGINE_GPUDEVICE_H
#define ZEROENGINE_GPUDEVICE_H

#include <memory>
#include <vector>

#include "zeroengine_core/ZERODefines.hpp"
#include "zeroengine_graphical/GPUContext.hpp"

namespace ZEROengine {
    class GPUDevice {
    protected:
        std::vector<std::shared_ptr<GraphicalContext>> m_graphical_contexts;

    public:
        virtual ZEROResult allocateBuffer() = 0;
        virtual ZEROResult allocateTexture() = 0;

        virtual std::weak_ptr<GraphicalContext> allocateGraphicalContext() = 0;

        virtual void cleanup() = 0;
    }; // class GPUDevice
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_GPUDEVICE_H