#include "Corium.h"
#include "CudaUtils.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Corium::Cuda::Utils {
	DeviceHandle DeviceHandle::makeCpu() {
		DeviceHandle handle;
		handle.m_HandleValue = CU_DEVICE_CPU;
		return handle;
	}

	uint32_t CudaHelpers::computeAllocFlag(std::initializer_list<HostAllocFlags> flags) {
		uint32_t mask = 0;
		for (const auto& each : flags)
			mask |= Internal::CUDA_InternalHelpers::toHostAllocationFlags(each);
		return mask;
	}

	uint32_t CudaHelpers::computeRegFlag(std::initializer_list<HostRegisterFlags> flags) {
		uint32_t mask = 0;
		for (const auto& each : flags)
			mask |= Internal::CUDA_InternalHelpers::toHostRegisterFlags(each);
		return mask;
	}

	uint64_t CudaHelpers::computeAccessFlags(std::initializer_list<AccessFlagBits> flags) {
		uint64_t result = 0;
		for (const auto& f : flags)
			result |= static_cast<uint64_t>(f);
		return result;
	}

	uint32_t CudaHelpers::computeEventFlags(std::initializer_list<EventFlags> flags) {
		uint32_t mask = 0;
		for (const auto& each : flags)
			mask |= Internal::CUDA_InternalHelpers::toEventFlags(each);
		return mask;
	}

	CORIUM_RUNTIME_API void initMemCpy3DDesc(MemCpy3DDesc& ro_Desc) {
		ro_Desc = MemCpy3DDesc{};
	}

	CORIUM_RUNTIME_API void setMemCpy3DSrcHost(MemCpy3DDesc& ro_Desc, PinnedAddress v_Src) {
		ro_Desc.m_SrcType = CopyMemoryType::HOST;
		ro_Desc.m_SrcHost = v_Src;
	}

	CORIUM_RUNTIME_API void setMemCpy3DSrcDevice(MemCpy3DDesc& ro_Desc, GpuAddress v_Src) {
		ro_Desc.m_SrcType   = CopyMemoryType::DEVICE;
		ro_Desc.m_SrcDevice = v_Src;
	}

	CORIUM_RUNTIME_API void setMemCpy3DSrcArray(MemCpy3DDesc& ro_Desc, CudaArray v_Src) {
		ro_Desc.m_SrcType  = CopyMemoryType::ARRAY;
		ro_Desc.m_SrcArray = v_Src;
	}

	CORIUM_RUNTIME_API void setMemCpy3DDstHost(MemCpy3DDesc& ro_Desc, PinnedAddress v_Dst) {
		ro_Desc.m_DstType = CopyMemoryType::HOST;
		ro_Desc.m_DstHost = v_Dst;
	}

	CORIUM_RUNTIME_API void setMemCpy3DDstDevice(MemCpy3DDesc& ro_Desc, GpuAddress v_Dst) {
		ro_Desc.m_DstType   = CopyMemoryType::DEVICE;
		ro_Desc.m_DstDevice = v_Dst;
	}

	CORIUM_RUNTIME_API void setMemCpy3DDstArray(MemCpy3DDesc& ro_Desc, CudaArray v_Dst) {
		ro_Desc.m_DstType  = CopyMemoryType::ARRAY;
		ro_Desc.m_DstArray = v_Dst;
	}

	CORIUM_RUNTIME_API void setMemCpy3DDimensions(MemCpy3DDesc& ro_Desc, size_t v_WidthInBytes, size_t v_Height, size_t v_Depth) {
		ro_Desc.m_WidthInBytes = v_WidthInBytes;
		ro_Desc.m_Height       = v_Height;
		ro_Desc.m_Depth        = v_Depth;
	}

	CORIUM_RUNTIME_API void setMemCpy3DSrcPitch(MemCpy3DDesc& ro_Desc, size_t v_Pitch, size_t v_Height) {
		ro_Desc.m_SrcPitch  = v_Pitch;
		ro_Desc.m_SrcHeight = v_Height;
	}

	CORIUM_RUNTIME_API void setMemCpy3DDstPitch(MemCpy3DDesc& ro_Desc, size_t v_Pitch, size_t v_Height) {
		ro_Desc.m_DstPitch  = v_Pitch;
		ro_Desc.m_DstHeight = v_Height;
	}

	CORIUM_RUNTIME_API void setMemCpy3DSrcOffsets(MemCpy3DDesc& ro_Desc, size_t v_XOffsetBytes, size_t v_YOffset, size_t v_ZOffset) {
		ro_Desc.m_SrcXOffsetBytes = v_XOffsetBytes;
		ro_Desc.m_SrcYOffset      = v_YOffset;
		ro_Desc.m_SrcZOffset      = v_ZOffset;
	}

	CORIUM_RUNTIME_API void setMemCpy3DDstOffsets(MemCpy3DDesc& ro_Desc, size_t v_XOffsetBytes, size_t v_YOffset, size_t v_ZOffset) {
		ro_Desc.m_DstXOffsetBytes = v_XOffsetBytes;
		ro_Desc.m_DstYOffset      = v_YOffset;
		ro_Desc.m_DstZOffset      = v_ZOffset;
	}

	CORIUM_RUNTIME_API void initAllocDesc(AllocDesc& ro_Desc) {
		ro_Desc.m_Type       = AllocationType::INVALID;
		ro_Desc.m_HandleType = AllocationHandleType::NONE;

		ro_Desc.m_Loc.m_Location = DeviceLocation::GPU;
		ro_Desc.m_Loc.m_Handle   = DeviceHandle{};

		ro_Desc.m_win32meta = nullptr;
	}

	CORIUM_RUNTIME_API void setAllocationType(AllocDesc& ro_Desc, AllocationType v_Type) {
		ro_Desc.m_Type = v_Type;
	}

	CORIUM_RUNTIME_API void setAllocationHandleType(AllocDesc& ro_Desc, AllocationHandleType v_Type) {
		ro_Desc.m_HandleType = v_Type;
	}

	CORIUM_RUNTIME_API void setLocation(AllocDesc& ro_Desc, DeviceHandle& ro_Handle) {
		ro_Desc.m_Loc.m_Location = DeviceLocation::GPU;
		ro_Desc.m_Loc.m_Handle   = ro_Handle;
	}

	CORIUM_RUNTIME_API void initAccessDesc(AccessDesc& ro_Desc) {
		ro_Desc.m_Loc.m_Location = DeviceLocation::GPU;
		ro_Desc.m_Loc.m_Handle   = DeviceHandle{};
		ro_Desc.flags            = 0;
	}

	CORIUM_RUNTIME_API void setAccessLocation(AccessDesc& ro_Desc, DeviceHandle& ro_Handle) {
		ro_Desc.m_Loc.m_Location = DeviceLocation::GPU;
		ro_Desc.m_Loc.m_Handle   = ro_Handle;
	}

	CORIUM_RUNTIME_API void setAccessFlags(AccessDesc& ro_Desc, uint64_t v_Flags) {
		ro_Desc.flags = v_Flags;
	}
}
