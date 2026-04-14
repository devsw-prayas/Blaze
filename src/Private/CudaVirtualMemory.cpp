#include "Corium.h"
#include "CudaVirtualMemory.h"

#define ALLOW_SYSCALL
#include "CoriumSyscalls.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Corium::Cuda::Memory {
    Utils::GpuAddress GpuVirtualMemory::reserveAddress(
        size_t v_Size,
        size_t v_Alignment,
        Utils::GpuAddress v_RequestedAddr
    ) {
        CORIUM_ASSERT(v_Size > 0);

        CUdeviceptr ptr{};
        Utils::GpuAddress addr{};

        const CUresult result = cuMemAddressReserve(
            &ptr,
            v_Size,
            v_Alignment,
            static_cast<CUdeviceptr>(v_RequestedAddr.m_GpuAddr),
            0
        );

        if (result == CUDA_SUCCESS) {
            addr.m_GpuAddr = ptr;
            return addr;
        }

        CUDA_ERROR_TRAP(result);
            return addr;
    }


    void GpuVirtualMemory::freeAddress(
        Utils::GpuAddress& ro_Address,
        size_t v_Size
    ) {
        if (!ro_Address.isValid()) return;

        const CUresult result = cuMemAddressFree(
            static_cast<CUdeviceptr>(ro_Address.m_GpuAddr),
            v_Size
        );

        if (result == CUDA_SUCCESS) {
            ro_Address.m_GpuAddr = 0;
            return;
        }

        CUDA_ERROR_TRAP(result);
    }

    Utils::AllocHandle GpuVirtualMemory::createAllocation(
        size_t v_Size,
        const Utils::AllocDesc& ro_Desc
    ) {
        CORIUM_ASSERT(v_Size > 0);

        CUmemAllocationProp prop{};
        prop.type = Internal::CUDA_InternalHelpers::toCuMemAllocationType(ro_Desc.m_Type);
        prop.requestedHandleTypes = Internal::CUDA_InternalHelpers::toCuMemAllocHandleType(ro_Desc.m_HandleType);

        prop.location.type = Internal::CUDA_InternalHelpers::toCUlocation(ro_Desc.m_Loc.m_Location);
        prop.location.id = ro_Desc.m_Loc.m_Handle.m_HandleValue;

#ifdef _WIN32
        prop.win32HandleMetaData = ro_Desc.m_win32meta;
#else
        prop.win32HandleMetaData = nullptr;
#endif

        CUmemGenericAllocationHandle handle{};
        Utils::AllocHandle outHandle{};

        const CUresult result = cuMemCreate(
            &handle,
            v_Size,
            &prop,
            0
        );

        if (result == CUDA_SUCCESS) {
            outHandle.m_Handle = handle;
            return outHandle;
        }

        CUDA_ERROR_TRAP(result);
            return outHandle;
    }


    void GpuVirtualMemory::releaseAllocation(
        Utils::AllocHandle& ro_Handle
    ) {
        if (!ro_Handle.isValid()) return;

        const CUresult result = cuMemRelease(
            static_cast<CUmemGenericAllocationHandle>(ro_Handle.m_Handle)
        );

        if (result == CUDA_SUCCESS) {
            ro_Handle.m_Handle = 0;
            return;
        }

        CUDA_ERROR_TRAP(result);
    }

    void GpuVirtualMemory::map(
        const Utils::GpuAddress& ro_Address,
        size_t v_Size,
        size_t v_Offset,
        const Utils::AllocHandle& ro_Handle
    ) {
        CORIUM_ASSERT(ro_Address.isValid());
        CORIUM_ASSERT(ro_Handle.isValid());
        CORIUM_ASSERT(v_Size > 0);

        const CUresult result = cuMemMap(
            static_cast<CUdeviceptr>(ro_Address.m_GpuAddr),
            v_Size,
            v_Offset,
            static_cast<CUmemGenericAllocationHandle>(ro_Handle.m_Handle),
            0
        );

        if (result == CUDA_SUCCESS) return;

        CUDA_ERROR_TRAP(result);
    }


    void GpuVirtualMemory::unmap(
        const Utils::GpuAddress& ro_Address,
        size_t v_Size
    ) {
        CORIUM_ASSERT(ro_Address.isValid());
        CORIUM_ASSERT(v_Size > 0);

        const CUresult result = cuMemUnmap(
            static_cast<CUdeviceptr>(ro_Address.m_GpuAddr),
            v_Size
        );

        if (result == CUDA_SUCCESS) return;

        CUDA_ERROR_TRAP(result);
    }

    void GpuVirtualMemory::setAccess(
        const Utils::GpuAddress& ro_Address,
        size_t v_Size,
        const Utils::AccessDesc* p_Desc,
        size_t v_Count
    ) {
        CORIUM_ASSERT(ro_Address.isValid());
        CORIUM_ASSERT(p_Desc != nullptr);
        CORIUM_ASSERT(v_Count > 0);

        // Stack allocate small array (typical usage = 1)
        CUmemAccessDesc descs[8]; // safe upper bound for now
        CORIUM_ASSERT(v_Count <= 8);

        for (size_t i = 0; i < v_Count; i++) {
            descs[i].location.type =
                Internal::CUDA_InternalHelpers::toCUlocation(p_Desc[i].m_Loc.m_Location);

            descs[i].location.id =
                p_Desc[i].m_Loc.m_Handle.m_HandleValue;

            descs[i].flags =
                Internal::CUDA_InternalHelpers::toAccessFlags(static_cast<Utils::AccessFlagBits>(p_Desc[i].flags));
        }

        const CUresult result = cuMemSetAccess(
            static_cast<CUdeviceptr>(ro_Address.m_GpuAddr),
            v_Size,
            descs,
            v_Count
        );

        if (result == CUDA_SUCCESS) return;

        CUDA_ERROR_TRAP(result);
    }

    // ------------------------------------------------------------
    // GRANULARITY
    // ------------------------------------------------------------

    size_t GpuVirtualMemory::getAllocationGranularity(
        const Utils::AllocDesc& ro_Desc,
        Utils::AllocationGranularityOption v_Option
    ) {
        CUmemAllocationProp prop{};
        prop.type = Internal::CUDA_InternalHelpers::toCuMemAllocationType(ro_Desc.m_Type);
        prop.requestedHandleTypes = Internal::CUDA_InternalHelpers::toCuMemAllocHandleType(ro_Desc.m_HandleType);

        prop.location.type = Internal::CUDA_InternalHelpers::toCUlocation(ro_Desc.m_Loc.m_Location);
        prop.location.id = ro_Desc.m_Loc.m_Handle.m_HandleValue;

#if defined(_WIN32)
        prop.win32HandleMetaData = ro_Desc.m_win32meta;
#else
        prop.win32HandleMetaData = nullptr;
#endif

        size_t granularity = 0;

        const CUresult result = cuMemGetAllocationGranularity(
            &granularity,
            &prop,
            Internal::CUDA_InternalHelpers::toCuMemAllocGranularity(v_Option)
        );

        if (result == CUDA_SUCCESS) return granularity;

        CUDA_ERROR_TRAP(result);
            return 0;
    }

    void GpuVirtualMemory::exportAllocation(
        void* p_Handle,
        const Utils::AllocHandle& ro_Handle,
        Utils::AllocationHandleType v_Type
    ) {
        CORIUM_ASSERT(p_Handle != nullptr);
        CORIUM_ASSERT(ro_Handle.isValid());

        const CUresult result = cuMemExportToShareableHandle(
            p_Handle,
            static_cast<CUmemGenericAllocationHandle>(ro_Handle.m_Handle),
            Internal::CUDA_InternalHelpers::toCuMemAllocHandleType(v_Type),
            0
        );

        if (result == CUDA_SUCCESS) return;

        CUDA_ERROR_TRAP(result);
    }

}