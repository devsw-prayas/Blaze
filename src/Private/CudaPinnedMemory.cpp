#include "Corium.h"
#include "CudaPinnedMemory.h"
#define ALLOW_SYSCALL
#include "CoriumSyscalls.h"
#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Corium::Cuda::Memory {

    PinnedAddress PinnedMemory::pinnedAlloc(size_t v_Bytes) {
        CORIUM_ASSERT(v_Bytes > 0);
        void* ptr = nullptr;
        const CUresult result = cuMemAllocHost(&ptr, v_Bytes);
        if (result == CUDA_SUCCESS)
            return PinnedAddress(ptr);
        CUDA_ERROR_TRAP(result);
            return PinnedAddress{};
    }

    PinnedAddress PinnedMemory::pinnedAlloc(size_t v_Bytes, uint32_t v_Flags) {
        CORIUM_ASSERT(v_Bytes > 0);
        void* ptr = nullptr;
        const CUresult result = cuMemHostAlloc(&ptr, v_Bytes, v_Flags);
        if (result == CUDA_SUCCESS)
            return PinnedAddress(ptr);
        CUDA_ERROR_TRAP(result);
            return PinnedAddress{};
    }

    void PinnedMemory::pinnedFree(PinnedAddress& ro_Addr) {
        if (!ro_Addr.isValid()) return;
        const CUresult result = cuMemFreeHost(ro_Addr.m_GpuAddr);
        if (result == CUDA_SUCCESS) {
            ro_Addr.m_GpuAddr = nullptr;
            return;
        }
        CUDA_ERROR_TRAP(result);
    }

    void PinnedMemory::mapToDevice(GpuAddress& ro_DevAddr, const PinnedAddress& ro_Addr, uint32_t v_Flags) {
        CORIUM_ASSERT(ro_Addr.isValid());
        CORIUM_ASSERT(v_Flags == 0); // CUDA spec: must be 0
        CUdeviceptr ptr{};
        const CUresult result = cuMemHostGetDevicePointer(&ptr, ro_Addr.m_GpuAddr, v_Flags);
        if (result == CUDA_SUCCESS) {
            ro_DevAddr.m_GpuAddr = ptr;
            return;
        }
        CUDA_ERROR_TRAP(result);
    }

    void PinnedMemory::hostMemRegister(const PinnedAddress& ro_Addr, size_t v_Bytes, uint32_t v_Flags) {
        CORIUM_ASSERT(ro_Addr.isValid());
        CORIUM_ASSERT(v_Bytes > 0);
        const CUresult result = cuMemHostRegister(ro_Addr.m_GpuAddr, v_Bytes, v_Flags);
        if (result == CUDA_SUCCESS) return;
        CUDA_ERROR_TRAP(result);
    }

    void PinnedMemory::hostMemUnRegister(const PinnedAddress& ro_Addr) {
        CORIUM_ASSERT(ro_Addr.isValid());
        const CUresult result = cuMemHostUnregister(ro_Addr.m_GpuAddr);
        if (result == CUDA_SUCCESS) return;
        CUDA_ERROR_TRAP(result);
    }

    void PinnedMemory::copyHostToDevAsync(const GpuAddress& ro_Dst, const PinnedAddress& ro_Src, size_t v_Bytes, const Streams::GpuStream& ro_Stream) {
        CORIUM_ASSERT(v_Bytes > 0);
        CORIUM_ASSERT(ro_Dst.isValid());
        CORIUM_ASSERT(ro_Src.isValid());
        CORIUM_ASSERT(ro_Stream.isValid());
        const CUresult result = cuMemcpyHtoDAsync(ro_Dst.m_GpuAddr, ro_Src.m_GpuAddr, v_Bytes, static_cast<CUstream>(ro_Stream.m_StreamHandle));
        if (result == CUDA_SUCCESS) return;
        CUDA_ERROR_TRAP(result);
    }

    void PinnedMemory::copyDevToHostAsync(const PinnedAddress& ro_Dst, const GpuAddress& ro_Src, size_t v_Bytes, const Streams::GpuStream& ro_Stream) {
        CORIUM_ASSERT(v_Bytes > 0);
        CORIUM_ASSERT(ro_Dst.isValid());
        CORIUM_ASSERT(ro_Src.isValid());
        CORIUM_ASSERT(ro_Stream.isValid());
        const CUresult result = cuMemcpyDtoHAsync(ro_Dst.m_GpuAddr, ro_Src.m_GpuAddr, v_Bytes, static_cast<CUstream>(ro_Stream.m_StreamHandle));
        if (result == CUDA_SUCCESS) return;
        CUDA_ERROR_TRAP(result);
    }
}