#include "Corium.h"
#include "CudaDeviceMemory.h"

#define ALLOW_SYSCALL
#include "CoriumSyscalls.h"

#define  ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Corium::Cuda::Memory {
	GpuAddress DeviceMemory::deviceAlloc(size_t v_Bytes) {
		CUdeviceptr ptr{};
		GpuAddress addr{};
		const CUresult result = cuMemAlloc(&ptr, v_Bytes);
		if (result == CUDA_SUCCESS) {
			addr.m_GpuAddr = ptr;
			return  addr;
		}
		CUDA_ERROR_TRAP(result);
			return addr;
	}

	PitchedAllocation DeviceMemory::deviceAllocPitch(size_t v_WidthInBytes, size_t v_Height, uint32_t v_ElemsInBytes) {
		PitchedAllocation alloc{};
		CUdeviceptr ptr{};
		size_t pitch = 0;
		const CUresult result = cuMemAllocPitch(&ptr, &pitch, v_WidthInBytes, v_Height, v_ElemsInBytes);
		if (result == CUDA_SUCCESS) {
			alloc.m_Address.m_GpuAddr = ptr;
			alloc.m_Pitch = pitch;
			return alloc;
		}
		CUDA_ERROR_TRAP(result);
			return alloc;
	}

	void DeviceMemory::deviceFree(GpuAddress& ro_Address) {
		if (!ro_Address.isValid()) return;
		const CUresult result = cuMemFree(static_cast<CUdeviceptr>(ro_Address.m_GpuAddr));
		if (result == CUDA_SUCCESS) {
			ro_Address.m_GpuAddr = 0;
			return;
		}
		CUDA_ERROR_TRAP(result);
	}

	GpuMemory DeviceMemory::query() {
		GpuMemory mem{};
		const CUresult result = cuMemGetInfo(&mem.m_AvailableMemory, &mem.m_TotalMemory);
		if (result == CUDA_SUCCESS) {
			return mem;
		}
		CUDA_ERROR_TRAP(result);
			return mem;
	}

	void DeviceMemory::memsetD8(const GpuAddress& ro_Address, uint8_t v_Val, size_t v_Count) {
		CORIUM_ASSERT(v_Count > 0);
		CORIUM_ASSERT(ro_Address.isValid());
		const CUresult result = cuMemsetD8(ro_Address.m_GpuAddr, v_Val, v_Count);
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result);
	}

	void DeviceMemory::memsetD16(const GpuAddress& ro_Address, uint16_t v_Val, size_t v_Count) {
		CORIUM_ASSERT(v_Count > 0);
		CORIUM_ASSERT(ro_Address.isValid());
		const CUresult result = cuMemsetD16(ro_Address.m_GpuAddr, v_Val, v_Count);
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result);
	}

	void DeviceMemory::memsetD32(const GpuAddress& ro_Address, uint32_t v_Val, size_t v_Count) {
		CORIUM_ASSERT(v_Count > 0);
		CORIUM_ASSERT(ro_Address.isValid());
		const CUresult result = cuMemsetD32(ro_Address.m_GpuAddr, v_Val, v_Count);
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result);
	}

	void DeviceMemory::memsetD8Async(const GpuAddress& ro_Address, uint8_t v_Val, size_t v_Count, const Streams::GpuStream& ro_Stream) {
		CORIUM_ASSERT(v_Count > 0);
		CORIUM_ASSERT(ro_Address.isValid());
		CORIUM_ASSERT(ro_Stream.isValid());
		const CUresult result = cuMemsetD8Async(ro_Address.m_GpuAddr, v_Val, v_Count, static_cast<CUstream>(ro_Stream.m_StreamHandle));
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result);
	}

	void DeviceMemory::memsetD16Async(const GpuAddress& ro_Address, uint16_t v_Val, size_t v_Count, const Streams::GpuStream& ro_Stream) {
		CORIUM_ASSERT(v_Count > 0);
		CORIUM_ASSERT(ro_Address.isValid());
		CORIUM_ASSERT(ro_Stream.isValid());
		const CUresult result = cuMemsetD16Async(ro_Address.m_GpuAddr, v_Val, v_Count, static_cast<CUstream>(ro_Stream.m_StreamHandle));
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result);
	}

	void DeviceMemory::memsetD32Async(const GpuAddress& ro_Address, uint32_t v_Val, size_t v_Count, const Streams::GpuStream& ro_Stream) {
		CORIUM_ASSERT(v_Count > 0);
		CORIUM_ASSERT(ro_Address.isValid());
		CORIUM_ASSERT(ro_Stream.isValid());
		const CUresult result = cuMemsetD32Async(ro_Address.m_GpuAddr, v_Val, v_Count, static_cast<CUstream>(ro_Stream.m_StreamHandle));
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result);
	}


	void DeviceMemory::copyDeviceToDevice(const GpuAddress& ro_Dst, const GpuAddress& ro_Src, size_t v_Bytes) {
		CORIUM_ASSERT(v_Bytes > 0);
		CORIUM_ASSERT(ro_Src.isValid());
		CORIUM_ASSERT(ro_Dst.isValid());
		const CUresult result = cuMemcpyDtoD(ro_Dst.m_GpuAddr, ro_Src.m_GpuAddr, v_Bytes);
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result);
	}

	void DeviceMemory::copyDeviceToDeviceAsync(const GpuAddress& ro_Dst, const GpuAddress& ro_Src, size_t v_Bytes, const Streams::GpuStream& ro_Stream) {
		CORIUM_ASSERT(v_Bytes > 0);
		CORIUM_ASSERT(ro_Src.isValid());
		CORIUM_ASSERT(ro_Dst.isValid());
		CORIUM_ASSERT(ro_Stream.isValid());
		const CUresult result = cuMemcpyDtoDAsync(ro_Dst.m_GpuAddr, ro_Src.m_GpuAddr, v_Bytes, static_cast<CUstream>(ro_Stream.m_StreamHandle));
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result);
	}
}
