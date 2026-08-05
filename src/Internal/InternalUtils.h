#pragma once
#include "Corium.h"
#include "ThreadUtils.h"
#include "CoriumMemory.h"

// Enum <-> raw value conversions, centralized here so no Public header ever
// needs to see OS or CUDA driver types. Definitions live in InternalUtils.cpp
// (behind ALLOW_SYSCALL) - never inline these in the header.
namespace Corium::Internal {
	int32_t toWin32Priority(Core::ThreadPriority v_Priority);
	Memory::MemState fromWin32MemState(uint32_t v_State);
}

#ifdef CORIUM_CUDA_AVAILABLE
#include "CudaUtils.h"

#define CUDA_ERROR_TRAP(result)	 \
	do {												 \
		CUresult res = (result);                        \
		if (res != CUDA_SUCCESS) {                      \
			const char* errorStr = nullptr;				\
			cuGetErrorString(res, &errorStr);		    \
			CORIUM_ASSERT(false && errorStr);		    \
			CORIUM_TRAP();						        \
		}                                               \
	} while(0)

#ifdef ALLOW_HELPERS
#include <cuda.h>

namespace Corium::Cuda::Internal {
	class CUDA_DeviceRegistry {
	public:
		static constexpr int MAX_DEVICE_COUNT = 32;
		static inline CUdevice s_Devices[MAX_DEVICE_COUNT] = {};
		static inline int s_DeviceCount = 0;
		static constexpr size_t CUDA_UUID_LENGTH = 16;

		static bool validateDevice(int v_Ordinal) {
			if (v_Ordinal >= s_DeviceCount || v_Ordinal < 0) return false;
			return true;
		}
	};

	class CUDA_InternalHelpers final {
	public:
		static CUdevice_attribute      toCudaAttr(Utils::CudaDeviceAttribute attr);
		static CUctx_flags_enum        toCudaContextScheduleFlags(Utils::ContextSchedulingFlags flag);
		static CUctx_flags_enum        toCudaContextCreationFlags(Utils::ContextCreationFlags flag);

		static uint32_t                toHostAllocationFlags(Utils::HostAllocFlags flag);
		static uint32_t                toHostRegisterFlags(Utils::HostRegisterFlags flag);

		static CUmemLocationType       toCUlocation(Utils::DeviceLocation flag);
		static CUmemAllocationType     toCuMemAllocationType(Utils::AllocationType flag);
		static CUmemAllocationHandleType toCuMemAllocHandleType(Utils::AllocationHandleType flag);

		static CUmemAccess_flags       toAccessFlags(Utils::AccessFlagBits flag);
		static CUmemAllocationGranularity_flags toCuMemAllocGranularity(Utils::AllocationGranularityOption v_Option);

		static CUmemorytype            toCopyMemoryType(Utils::CopyMemoryType v_Type);
		static CUstreamCaptureMode     toCudaStreamCaptureMode(Utils::StreamCaptureMode v_Mode);
		static CUstreamCaptureStatus   toCudaStreamCaptureStatus(Utils::StreamCaptureStatus v_Status);
		static uint32_t                toStreamFlags(Utils::StreamFlags v_Flags);
		static uint32_t                toEventFlags(Utils::EventFlags v_Flags);
	};

	class CUDA_PackingFunctions final {
	public:
		static CUDA_MEMCPY3D           pack3dMemcpyDesc(const Utils::MemCpy3DDesc& ro_Desc);        // → cuMemcpy3DAsync, cuGraphAddMemcpyNode
		static CUDA_KERNEL_NODE_PARAMS packKernelNodeParams(const Utils::KernelNodeParams& ro_Params); // → cuGraphAddKernelNode
		static CUDA_MEMSET_NODE_PARAMS packMemsetNodeParams(const Utils::MemsetNodeParams& ro_Params); // → cuGraphAddMemsetNode
	};
}

#endif // ALLOW_HELPERS
#endif // CORIUM_CUDA_AVAILABLE
