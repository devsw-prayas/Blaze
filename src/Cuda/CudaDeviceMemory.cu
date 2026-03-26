#include "Corium.h"
#include "CudaDeviceMemory.h"
#define ALLOW_SYSCALLS
#include "CoriumSyscalls.h"

#ifdef  CORIUM_CUDA_AVAILABLE
namespace Corium::Cuda {
	DeviceSegment DeviceMemory::deviceAlloc(DeviceSegment& ro_DevSegment, Bytes v_Size) {
		cudaError_t error = cudaMalloc(&ro_DevSegment.m_DevicePtr, v_Size);
		if (error == cudaSuccess) {
			ro_DevSegment.m_TotalSize = v_Size;
			ro_DevSegment.m_CommittedSize = 0;
			return ro_DevSegment;
		}
		return INVALID_DEVICE_SEGMENT;
	}

	bool DeviceMemory::deviceFree(DeviceSegment& ro_DevSegment) {
		cudaError_t error = cudaFree(ro_DevSegment.m_DevicePtr);
		if (error == cudaSuccess) {
			ro_DevSegment.m_DevicePtr = nullptr;
			ro_DevSegment.m_TotalSize = 0;
			ro_DevSegment.m_CommittedSize = 0;
			return true;
		}
		return false;
	}

	PinnedSegment DeviceMemory::pinnedAlloc(PinnedSegment& ro_PinSegment, Bytes v_Size) {
		cudaError_t error = cudaMallocHost(&ro_PinSegment.m_HostPtr, v_Size);
		if (error == cudaSuccess) {
			ro_PinSegment.m_TotalSize = v_Size;
			return ro_PinSegment;
		}
		return INVALID_PINNED_SEGMENT;
	}

	bool DeviceMemory::pinnedFree(PinnedSegment& ro_PinSegment) {
		cudaError_t error = cudaFreeHost(ro_PinSegment.m_HostPtr);
		if (error == cudaSuccess) {
			ro_PinSegment.m_TotalSize = 0;
			return true;
		}
		return false;
	}

	bool DeviceMemory::copyToDevice(DeviceSegment& ro_DevDst, const PinnedSegment& ro_PinSrc, Bytes v_Size, void* po_CudaStream) {
		cudaStream_t stream = *static_cast<cudaStream_t*>(po_CudaStream);
		auto error = cudaMemcpyAsync(ro_DevDst.m_DevicePtr, ro_PinSrc.m_HostPtr, v_Size, cudaMemcpyHostToDevice, stream);
		if (error == cudaSuccess) return true;
		return false;
	}

	bool DeviceMemory::copyToHost(PinnedSegment& ro_PinDst, const DeviceSegment& ro_DevSrc, Bytes v_Size, void* po_CudaStream) {
		cudaStream_t stream = *static_cast<cudaStream_t*>(po_CudaStream);
		auto error = cudaMemcpyAsync(ro_PinDst.m_HostPtr, ro_DevSrc.m_DevicePtr, v_Size, cudaMemcpyDeviceToHost, stream);
		if (error == cudaSuccess) return true;
		return false;
	}

	bool DeviceMemory::deviceMemset(DeviceSegment& ro_DevSegment, int v_Val, Bytes v_Size, void* po_CudaStream) {
		cudaStream_t stream = *static_cast<cudaStream_t*>(po_CudaStream);
		auto error = cudaMemsetAsync(ro_DevSegment.m_DevicePtr, v_Val, v_Size, stream);
		if (error == cudaSuccess) return true;
		return false;
	}

	Bytes DeviceMemory::queryFreeMemory() {
		size_t free = 0, total = 0;
		if (cudaMemGetInfo(&free, &total) == cudaSuccess) return free;
		return 0;
	}
}
#endif