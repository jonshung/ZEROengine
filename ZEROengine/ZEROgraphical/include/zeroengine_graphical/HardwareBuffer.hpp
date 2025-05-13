#ifndef ZEROENGINE_HARDWARE_BUFFER_H
#define ZEROENGINE_HARDWARE_BUFFER_H

namespace ZEROengine {
    class HardwareBuffer {
    protected:
        void *m_p_buffer;
    
    public:
    HardwareBuffer();
    virtual ~HardwareBuffer();
    }; // class HardwareBuffer
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_HARDWARE_BUFFER_H