#pragma once
#include "Corium.h"
#include "CudaUtils.h"

#define CUDA_ERROR_TRAP(result)	 \
	do{												 \
		const char* errorStr = nullptr;				   \
		cuGetErrorString(result, &errorStr);		   \
		CORIUM_ASSERT(false && errorStr);		   \
		CORIUM_TRAP();						   \
	}while(0); 										   \

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
	};
}

#else
#error "This is an internal backend header. To use, define ALLOW_HELPERS before inclusion"
#endif
