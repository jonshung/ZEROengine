#ifndef ZEROENGINE_HARDWAREBUFFER_H
#define ZEROENGINE_HARDWAREBUFFER_H

namespace ZEROengine {
    class HardwareBuffer {
    protected:
        void *m_p_buffer;
    
    public:
    HardwareBuffer();
    virtual ~HardwareBuffer();
    }; // class HardwareBuffer
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_HARDWAREBUFFER_H