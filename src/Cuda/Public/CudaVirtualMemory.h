#pragma once
#include "CudaStream.h"
#include "Corium.h"
#include "CoriumCompiler.h"
#include "CoriumDiagnostics.h"
#include "CudaUtils.h"

namespace Corium::Cuda::Memory {
	using namespace Utils;

	class CORIUM_RUNTIME_API GpuVirtualMemory final {
	public:
		static GpuAddress reserveAddress(
			size_t v_Size, size_t v_Alignment = 0,
			GpuAddress v_RequestedAddr = {}
		);

		static void freeAddress(
			GpuAddress& ro_Address,
			size_t v_Size
		);

		static AllocHandle createAllocation(
			size_t v_Size,
			const AllocDesc& ro_Desc
		);

		static void releaseAllocation(
			AllocHandle& ro_Handle
		);

		static void map(
			const GpuAddress& ro_Address,
			size_t v_Size,
			size_t v_Offset,
			const AllocHandle& ro_Handle
		);

		static void unmap(
			const GpuAddress& ro_Address, size_t v_Size
		);

		static void setAccess(
			const GpuAddress& ro_Address, size_t v_Size,
			const AccessDesc* p_Desc, size_t v_Count
		);

		static size_t getAllocationGranularity(
			const AllocDesc& ro_Desc, AllocationGranularityOption v_Option
		);

		static void exportAllocation(
			void* p_Handle, const AllocHandle& ro_Handle, AllocationHandleType v_Type
		);
	};
}
