#ifndef ZEROENGINE_GPUSYNCPRIMITIVES_H
#define ZEROENGINE_GPUSYNCPRIMITIVES_H

namespace ZEROengine {
    class GPUSyncPrimitive {
    protected:
        void* m_api_semaphore_handle; // purely GPU-to-GPU sync primitives
        void* m_api_fence_handle; // purely CPU-to-GPU sync primitivies
    
    public:
        virtual void waitOnFence() = 0;
        virtual bool getFenceStatus() = 0;
        virtual void releaseFence() = 0;

        virtual void* getSemaphore() = 0;
    }; // class GPUSyncPrimitive
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_GPUSYNCPRIMITIVES_H