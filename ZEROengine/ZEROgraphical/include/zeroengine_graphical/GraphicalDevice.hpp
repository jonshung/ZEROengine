#ifndef ZEROENGINE_GRAPHICALDEVICE_H
#define ZEROENGINE_GRAPHICALDEVICE_H

#include "zeroengine_core/ZERODefines.hpp"

namespace ZEROengine {
    class GraphicalDevice {
    public:
        virtual ZEROResult allocateBuffer() = 0;
        virtual ZEROResult allocateTexture() = 0;

        virtual void cleanup() = 0;
    }; // class GraphicalDevice
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_GRAPHICALDEVICE_H