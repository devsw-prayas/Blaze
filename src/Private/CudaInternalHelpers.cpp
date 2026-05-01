#include "Corium.h"
#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

#ifdef ALLOW_HELPERS
namespace Corium::Cuda::Internal {
	CUdevice_attribute CUDA_InternalHelpers::toCudaAttr(Utils::CudaDeviceAttribute attr) {
		switch (attr) {
		case Utils::CudaDeviceAttribute::COMPUTE_CAPABILITY_MAJOR:
			return CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR;

		case Utils::CudaDeviceAttribute::COMPUTE_CAPABILITY_MINOR:
			return CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR;

		case Utils::CudaDeviceAttribute::MAX_THREADS_PER_BLOCK:
			return CU_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK;

		case Utils::CudaDeviceAttribute::MAX_GRID_DIM_X:
			return CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X;

		case Utils::CudaDeviceAttribute::MAX_GRID_DIM_Y:
			return CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y;

		case Utils::CudaDeviceAttribute::MAX_GRID_DIM_Z:
			return CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Z;

		case Utils::CudaDeviceAttribute::MAX_SHARED_MEMORY_PER_BLOCK:
			return CU_DEVICE_ATTRIBUTE_MAX_SHARED_MEMORY_PER_BLOCK;

		case Utils::CudaDeviceAttribute::WARP_SIZE:
			return CU_DEVICE_ATTRIBUTE_WARP_SIZE;

		case Utils::CudaDeviceAttribute::MEMORY_CLOCK_RATE:
			return CU_DEVICE_ATTRIBUTE_MEMORY_CLOCK_RATE;

		case Utils::CudaDeviceAttribute::GLOBAL_MEMORY_BUS_WIDTH:
			return CU_DEVICE_ATTRIBUTE_GLOBAL_MEMORY_BUS_WIDTH;

		case Utils::CudaDeviceAttribute::L2_CACHE_SIZE:
			return CU_DEVICE_ATTRIBUTE_L2_CACHE_SIZE;

		case Utils::CudaDeviceAttribute::UNIFIED_ADDRESSING:
			return CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING;

		case Utils::CudaDeviceAttribute::CONCURRENT_KERNELS:
			return CU_DEVICE_ATTRIBUTE_CONCURRENT_KERNELS;

		case Utils::CudaDeviceAttribute::CAN_USE_HOST_POINTER_FOR_REGISTERED_MEM:
			return CU_DEVICE_ATTRIBUTE_CAN_USE_HOST_POINTER_FOR_REGISTERED_MEM;

		case Utils::CudaDeviceAttribute::GPU_DIRECT_RDMA_SUPPORTED:
			return CU_DEVICE_ATTRIBUTE_GPU_DIRECT_RDMA_SUPPORTED;

		case Utils::CudaDeviceAttribute::VIRTUAL_MEMORY_MANAGEMENT_SUPPORTED:
			return CU_DEVICE_ATTRIBUTE_VIRTUAL_MEMORY_MANAGEMENT_SUPPORTED;

		case Utils::CudaDeviceAttribute::CONCURRENT_MANAGED_ACCESS:
			return CU_DEVICE_ATTRIBUTE_CONCURRENT_MANAGED_ACCESS;
		}
		CORIUM_ASSERT(false && "Invalid CudaDeviceAttribute");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	CUctx_flags_enum CUDA_InternalHelpers::toCudaContextScheduleFlags(Utils::ContextSchedulingFlags flag) {
		switch (flag) {
		case Utils::ContextSchedulingFlags::SCHEDULE_AUTO:          return CU_CTX_SCHED_AUTO;
		case Utils::ContextSchedulingFlags::SCHEDULE_SPIN:          return CU_CTX_SCHED_SPIN;
		case Utils::ContextSchedulingFlags::SCHEDULE_YIELD:         return CU_CTX_SCHED_YIELD;
		case Utils::ContextSchedulingFlags::SCHEDULE_BLOCKING_SYNC: return CU_CTX_BLOCKING_SYNC;
		}
		CORIUM_ASSERT(false && "Invalid Context Scheduling Flags");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	CUctx_flags_enum CUDA_InternalHelpers::toCudaContextCreationFlags(Utils::ContextCreationFlags flag) {
		switch (flag) {
		case Utils::ContextCreationFlags::NONE:              return static_cast<CUctx_flags_enum>(0);
		case Utils::ContextCreationFlags::MAP_HOST:          return CU_CTX_MAP_HOST;
		case Utils::ContextCreationFlags::LMEM_RESIZE_TO_MAX: return CU_CTX_LMEM_RESIZE_TO_MAX;
		}
		CORIUM_ASSERT(false && "Invalid Context Creation Flag");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	uint32_t CUDA_InternalHelpers::toHostAllocationFlags(Utils::HostAllocFlags flag) {
		switch (flag) {
		case Utils::HostAllocFlags::ALLOC_DEVICE_MAP:     return CU_MEMHOSTALLOC_DEVICEMAP;
		case Utils::HostAllocFlags::ALLOC_PORTABLE:       return CU_MEMHOSTALLOC_PORTABLE;
		case Utils::HostAllocFlags::ALLOC_WRITE_COMBINED: return CU_MEMHOSTALLOC_WRITECOMBINED;
		}
		CORIUM_ASSERT(false && "Invalid Host Alloc Flags");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	uint32_t CUDA_InternalHelpers::toHostRegisterFlags(Utils::HostRegisterFlags flag) {
		switch (flag) {
		case Utils::HostRegisterFlags::REG_DEVICE_MAP: return CU_MEMHOSTREGISTER_DEVICEMAP;
		case Utils::HostRegisterFlags::REG_IO_MEMORY:  return CU_MEMHOSTREGISTER_IOMEMORY;
		case Utils::HostRegisterFlags::REG_PORTABLE:   return CU_MEMHOSTREGISTER_PORTABLE;
		case Utils::HostRegisterFlags::REG_READ_ONLY:  return CU_MEMHOSTREGISTER_READ_ONLY;
		}
		CORIUM_ASSERT(false && "Invalid Host Register Flags");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	CUmemLocationType CUDA_InternalHelpers::toCUlocation(Utils::DeviceLocation flag) {
		switch (flag) {
		case Utils::DeviceLocation::CPU: return CU_MEM_LOCATION_TYPE_HOST;
		case Utils::DeviceLocation::GPU: return CU_MEM_LOCATION_TYPE_DEVICE;
		}
		CORIUM_ASSERT(false && "Invalid Device location");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	CUmemAllocationType CUDA_InternalHelpers::toCuMemAllocationType(Utils::AllocationType flag) {
		switch (flag) {
		case Utils::AllocationType::INVALID: return CU_MEM_ALLOCATION_TYPE_INVALID;
		case Utils::AllocationType::PINNED:  return CU_MEM_ALLOCATION_TYPE_PINNED;
		}
		CORIUM_ASSERT(false && "Invalid Allocation Type");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	CUmemAllocationHandleType CUDA_InternalHelpers::toCuMemAllocHandleType(Utils::AllocationHandleType flag) {
		switch (flag) {
		case Utils::AllocationHandleType::NONE:           return CU_MEM_HANDLE_TYPE_NONE;
		case Utils::AllocationHandleType::WIN32_HANDLE:   return CU_MEM_HANDLE_TYPE_WIN32;
		case Utils::AllocationHandleType::FABRIC_HANDLE:  return CU_MEM_HANDLE_TYPE_FABRIC;
		}
		CORIUM_ASSERT(false && "Invalid Handle type");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	CUmemAllocationGranularity_flags CUDA_InternalHelpers::toCuMemAllocGranularity(Utils::AllocationGranularityOption v_Option) {
		switch (v_Option) {
		case Utils::AllocationGranularityOption::MINIMUM:     return CU_MEM_ALLOC_GRANULARITY_MINIMUM;
		case Utils::AllocationGranularityOption::RECOMMENDED: return CU_MEM_ALLOC_GRANULARITY_RECOMMENDED;
		}
		CORIUM_ASSERT(false && "Invalid Allocation Granularity Option");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	CUmemAccess_flags CUDA_InternalHelpers::toAccessFlags(Utils::AccessFlagBits flag) {
		switch (flag) {
		case Utils::AccessFlagBits::READ:      return CU_MEM_ACCESS_FLAGS_PROT_READ;
		case Utils::AccessFlagBits::READWRITE: return CU_MEM_ACCESS_FLAGS_PROT_READWRITE;
		case Utils::AccessFlagBits::NONE:      return CU_MEM_ACCESS_FLAGS_PROT_NONE;
		}
		CORIUM_ASSERT(false && "Invalid Access Flag");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	CUmemorytype CUDA_InternalHelpers::toCopyMemoryType(Utils::CopyMemoryType v_Type) {
		switch (v_Type) {
		case Utils::CopyMemoryType::HOST:   return CU_MEMORYTYPE_HOST;
		case Utils::CopyMemoryType::DEVICE: return CU_MEMORYTYPE_DEVICE;
		case Utils::CopyMemoryType::ARRAY:  return CU_MEMORYTYPE_ARRAY;
		}
		CORIUM_ASSERT(false && "Invalid CopyMemoryType");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	CUstreamCaptureMode CUDA_InternalHelpers::toCudaStreamCaptureMode(Utils::StreamCaptureMode v_Mode) {
		switch (v_Mode) {
		case Utils::StreamCaptureMode::GLOBAL:       return CU_STREAM_CAPTURE_MODE_GLOBAL;
		case Utils::StreamCaptureMode::THREAD_LOCAL: return CU_STREAM_CAPTURE_MODE_THREAD_LOCAL;
		case Utils::StreamCaptureMode::RELAXED:      return CU_STREAM_CAPTURE_MODE_RELAXED;
		}
		CORIUM_ASSERT(false && "Invalid StreamCaptureMode");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	CUstreamCaptureStatus CUDA_InternalHelpers::toCudaStreamCaptureStatus(Utils::StreamCaptureStatus v_Status) {
		switch (v_Status) {
		case Utils::StreamCaptureStatus::NONE:        return CU_STREAM_CAPTURE_STATUS_NONE;
		case Utils::StreamCaptureStatus::ACTIVE:      return CU_STREAM_CAPTURE_STATUS_ACTIVE;
		case Utils::StreamCaptureStatus::INVALIDATED: return CU_STREAM_CAPTURE_STATUS_INVALIDATED;
		}
		CORIUM_ASSERT(false && "Invalid StreamCaptureStatus");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	uint32_t CUDA_InternalHelpers::toStreamFlags(Utils::StreamFlags v_Flags) {
		switch (v_Flags) {
		case Utils::StreamFlags::DEFAULT:      return 0;
		case Utils::StreamFlags::NON_BLOCKING: return CU_STREAM_NON_BLOCKING;
		}
		CORIUM_ASSERT(false && "Invalid StreamFlags");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	uint32_t CUDA_InternalHelpers::toEventFlags(Utils::EventFlags v_Flags) {
		switch (v_Flags) {
		case Utils::EventFlags::DEFAULT:        return CU_EVENT_DEFAULT;
		case Utils::EventFlags::BLOCKING_SYNC:  return CU_EVENT_BLOCKING_SYNC;
		case Utils::EventFlags::DISABLE_TIMING: return CU_EVENT_DISABLE_TIMING;
		case Utils::EventFlags::INTERPROCESS:   return CU_EVENT_INTERPROCESS;
		}
		CORIUM_ASSERT(false && "Invalid EventFlags");
		CORIUM_TRAP();
		CORIUM_UNREACHABLE();
	}

	CUDA_MEMCPY3D CUDA_PackingFunctions::pack3dMemcpyDesc(const Utils::MemCpy3DDesc& ro_Desc) {
		CUDA_MEMCPY3D desc{};

		desc.srcMemoryType = CUDA_InternalHelpers::toCopyMemoryType(ro_Desc.m_SrcType);
		desc.srcHost       = ro_Desc.m_SrcHost.m_GpuAddr;
		desc.srcDevice     = ro_Desc.m_SrcDevice.m_GpuAddr;
		desc.srcArray      = static_cast<CUarray>(ro_Desc.m_SrcArray.m_Array);
		desc.srcXInBytes   = ro_Desc.m_SrcXOffsetBytes;
		desc.srcY          = ro_Desc.m_SrcYOffset;
		desc.srcZ          = ro_Desc.m_SrcZOffset;
		desc.srcPitch      = ro_Desc.m_SrcPitch;
		desc.srcHeight     = ro_Desc.m_SrcHeight;

		desc.dstMemoryType = CUDA_InternalHelpers::toCopyMemoryType(ro_Desc.m_DstType);
		desc.dstHost       = ro_Desc.m_DstHost.m_GpuAddr;
		desc.dstDevice     = ro_Desc.m_DstDevice.m_GpuAddr;
		desc.dstArray      = static_cast<CUarray>(ro_Desc.m_DstArray.m_Array);
		desc.dstXInBytes   = ro_Desc.m_DstXOffsetBytes;
		desc.dstY          = ro_Desc.m_DstYOffset;
		desc.dstZ          = ro_Desc.m_DstZOffset;
		desc.dstPitch      = ro_Desc.m_DstPitch;
		desc.dstHeight     = ro_Desc.m_DstHeight;

		desc.WidthInBytes  = ro_Desc.m_WidthInBytes;
		desc.Height        = ro_Desc.m_Height;
		desc.Depth         = ro_Desc.m_Depth;

		return desc;
	}

	CUDA_KERNEL_NODE_PARAMS CUDA_PackingFunctions::packKernelNodeParams(const Utils::KernelNodeParams& ro_Params) {
		CUDA_KERNEL_NODE_PARAMS params{};
		params.func           = static_cast<CUfunction>(ro_Params.m_Function);
		params.gridDimX       = ro_Params.m_GridDimX;
		params.gridDimY       = ro_Params.m_GridDimY;
		params.gridDimZ       = ro_Params.m_GridDimZ;
		params.blockDimX      = ro_Params.m_BlockDimX;
		params.blockDimY      = ro_Params.m_BlockDimY;
		params.blockDimZ      = ro_Params.m_BlockDimZ;
		params.sharedMemBytes = ro_Params.m_SharedMemBytes;
		params.kernelParams   = ro_Params.m_KernelParams;
		params.extra          = ro_Params.m_Extra;
		return params;
	}

	CUDA_MEMSET_NODE_PARAMS CUDA_PackingFunctions::packMemsetNodeParams(const Utils::MemsetNodeParams& ro_Params) {
		CUDA_MEMSET_NODE_PARAMS params{};
		params.dst         = static_cast<CUdeviceptr>(ro_Params.m_Dst);
		params.pitch       = ro_Params.m_Pitch;
		params.value       = ro_Params.m_Value;
		params.elementSize = ro_Params.m_ElementSize;
		params.width       = ro_Params.m_Width;
		params.height      = ro_Params.m_Height;
		return params;
	}
}
#endif
