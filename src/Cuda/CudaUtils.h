#pragma once

#include "CoriumCompiler.h"
#include "CoriumDiagnostics.h"

namespace Corium::Cuda::Utils {
	enum class CORIUM_RUNTIME_API CudaDeviceAttribute : uint8_t {
		COMPUTE_CAPABILITY_MAJOR,               // Major SM version (architecture generation)
		COMPUTE_CAPABILITY_MINOR,               // Minor SM version (architecture revision)

		MAX_THREADS_PER_BLOCK,                  // Maximum number of threads in a single block
		MAX_GRID_DIM_X,                         // Maximum grid dimension in X direction
		MAX_GRID_DIM_Y,                         // Maximum grid dimension in Y direction
		MAX_GRID_DIM_Z,                         // Maximum grid dimension in Z direction

		MAX_SHARED_MEMORY_PER_BLOCK,            // Maximum shared memory available per block (bytes)

		WARP_SIZE,                              // Number of threads per warp (typically 32)

		MEMORY_CLOCK_RATE,                      // Memory clock frequency (kHz)
		GLOBAL_MEMORY_BUS_WIDTH,                // Width of memory bus (bits)

		L2_CACHE_SIZE,                          // Size of L2 cache (bytes)

		UNIFIED_ADDRESSING,                     // Whether unified virtual addressing is supported (0/1)
		CONCURRENT_KERNELS,                     // Whether multiple kernels can execute concurrently (0/1)

		CAN_USE_HOST_POINTER_FOR_REGISTERED_MEM,// Whether host pointers can be directly used after registration (0/1)

		GPU_DIRECT_RDMA_SUPPORTED,              // Support for GPUDirect RDMA (0/1)
		VIRTUAL_MEMORY_MANAGEMENT_SUPPORTED,    // Support for CUDA virtual memory APIs (0/1)
		CONCURRENT_MANAGED_ACCESS               // GPU can access managed memory concurrently with CPU (0/1); false on Windows WDDM
	};

	enum class CORIUM_RUNTIME_API ContextSchedulingFlags final : uint8_t {
		SCHEDULE_AUTO,
		SCHEDULE_SPIN,
		SCHEDULE_YIELD,
		SCHEDULE_BLOCKING_SYNC
	};

	enum class CORIUM_RUNTIME_API ContextCreationFlags final : uint8_t {
		NONE,
		MAP_HOST,
		LMEM_RESIZE_TO_MAX
	};

	enum class CORIUM_RUNTIME_API HostAllocFlags final : uint8_t {
		ALLOC_PORTABLE,
		ALLOC_DEVICE_MAP,
		ALLOC_WRITE_COMBINED
	};

	enum class CORIUM_RUNTIME_API HostRegisterFlags final : uint8_t {
		REG_PORTABLE,
		REG_DEVICE_MAP,
		REG_IO_MEMORY,
		REG_READ_ONLY
	};

	enum class DeviceLocation final : uint8_t {
		CPU, GPU
	};

	enum class CORIUM_RUNTIME_API AllocationType final : uint8_t {
		INVALID,
		PINNED,
	};

	enum class CORIUM_RUNTIME_API AllocationHandleType final : uint8_t {
		NONE,
		WIN32_HANDLE,
		FABRIC_HANDLE
	};

	enum class CORIUM_RUNTIME_API AccessFlagBits : uint8_t {
		NONE      = 0,
		READ      = 1 << 0,
		READWRITE = 1 << 1
	};

	enum class CORIUM_RUNTIME_API AllocationGranularityOption final : uint8_t {
		MINIMUM,
		RECOMMENDED
	};

	class CORIUM_RUNTIME_API CudaHelpers final {
	public:
		static uint32_t computeAllocFlag(std::initializer_list<HostAllocFlags> flags);
		static uint32_t computeRegFlag(std::initializer_list<HostRegisterFlags> flags);
		static uint64_t computeAccessFlags(std::initializer_list<AccessFlagBits> flags);
	};

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(4) DeviceHandle final {
		int m_HandleValue;

		bool isValid() const {
			return m_HandleValue > -1;
		}
		DeviceHandle() = default;
		~DeviceHandle() = default;

		DeviceHandle(const DeviceHandle&) = default;
		DeviceHandle& operator=(const DeviceHandle&) = default;

		DeviceHandle(DeviceHandle&&) noexcept = default;
		DeviceHandle& operator=(DeviceHandle&&) noexcept = default;

		static DeviceHandle makeCpu();
	};

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(16) DeviceUUID final {
		uint64_t m_Lo;
		uint64_t m_Hi;

		DeviceUUID() = default;
		~DeviceUUID() = default;

		DeviceUUID(const DeviceUUID&) = default;
		DeviceUUID& operator=(const DeviceUUID&) = default;

		DeviceUUID(DeviceUUID&&) noexcept = default;
		DeviceUUID& operator=(DeviceUUID&&) noexcept = default;
	};

	CORIUM_STATIC_ASSERT(sizeof(DeviceUUID) == 16, "Inavlid UUID Struct layout size, must be 16");
	CORIUM_STATIC_ASSERT(std::is_standard_layout_v<DeviceUUID>, "Invalid UUID struct layout, must maintain standard layout");
	CORIUM_STATIC_ASSERT(std::is_trivially_copyable_v<DeviceUUID>, "Invalid UUID struct members, must be trivial");
	CORIUM_STATIC_ASSERT(std::is_trivially_move_assignable_v<DeviceUUID>, "Invalid UUID struct members, must be trivial");

	using CtxPtr = void*;

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(8) CudaContext final {
		CtxPtr m_Handle;

		CudaContext() = default;
		~CudaContext() = default;

		CudaContext(const CudaContext&) = default;
		CudaContext& operator=(const CudaContext&) = default;

		CudaContext(CudaContext&&) noexcept = default;
		CudaContext& operator=(CudaContext&&) noexcept = default;
	};

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(8) PinnedAddress final {
		void* m_GpuAddr = nullptr;

		explicit PinnedAddress(void* addr) : m_GpuAddr(addr) {}
		PinnedAddress() = default;
		~PinnedAddress() = default;

		PinnedAddress(const PinnedAddress&) = default;
		PinnedAddress& operator=(const PinnedAddress&) = default;

		PinnedAddress(PinnedAddress&&) noexcept = default;
		PinnedAddress& operator=(PinnedAddress&&) noexcept = default;

		CORIUM_NODISCARD bool isValid() const {
			return m_GpuAddr != nullptr;
		}
	};

	CORIUM_STATIC_ASSERT(std::is_standard_layout_v<PinnedAddress>, "GpuAddress must maintain standard layout");
	CORIUM_STATIC_ASSERT(std::is_trivially_copyable_v<PinnedAddress>, "GpuAddress must be trivially copyable");
	CORIUM_STATIC_ASSERT(std::is_trivially_move_assignable_v<PinnedAddress>, "GpuAddress must be trivially move assignable");
	CORIUM_STATIC_ASSERT(sizeof(PinnedAddress) == 8, "Invalid GpuAddress size, must be 64bit");

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(16) GpuMemory final {
		size_t m_TotalMemory = 0;
		size_t m_AvailableMemory = 0;

		GpuMemory() = default;
		~GpuMemory() = default;

		GpuMemory(const GpuMemory&) = default;
		GpuMemory& operator=(const GpuMemory&) = default;

		GpuMemory(GpuMemory&&) noexcept = default;
		GpuMemory& operator=(GpuMemory&&) noexcept = default;
	};

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(8) GpuAddress final {
		uint64_t m_GpuAddr = 0;

		explicit GpuAddress(uint64_t addr) : m_GpuAddr(addr) {}
		GpuAddress() = default;
		~GpuAddress() = default;

		GpuAddress(const GpuAddress&) = default;
		GpuAddress& operator=(const GpuAddress&) = default;

		GpuAddress(GpuAddress&&) noexcept = default;
		GpuAddress& operator=(GpuAddress&&) noexcept = default;

		CORIUM_NODISCARD bool isValid() const {
			return m_GpuAddr != 0;
		}
	};

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(8) AllocHandle final {
		uint64_t m_Handle;

		bool isValid() const {
			return m_Handle ? 1 : 0;
		}

		AllocHandle() = default;
		~AllocHandle() = default;

		AllocHandle(const AllocHandle&) = default;
		AllocHandle& operator=(const AllocHandle&) = default;

		AllocHandle(AllocHandle&&) noexcept = default;
		AllocHandle& operator=(AllocHandle&&) noexcept = default;
	};

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(8) Location final {
		DeviceLocation m_Location;
		DeviceHandle   m_Handle;

		Location() = default;
		~Location() = default;

		Location(const Location&) = default;
		Location& operator=(const Location&) = default;

		Location(Location&&) noexcept = default;
		Location& operator=(Location&&) noexcept = default;
	};

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(16) AllocDesc final {
		void*                m_win32meta;
		Location             m_Loc;
		AllocationType       m_Type;
		AllocationHandleType m_HandleType;

		AllocDesc() = default;
		~AllocDesc() = default;

		AllocDesc(const AllocDesc&) = default;
		AllocDesc& operator=(const AllocDesc&) = default;

		AllocDesc(AllocDesc&&) noexcept = default;
		AllocDesc& operator=(AllocDesc&&) noexcept = default;
	};

	CORIUM_RUNTIME_API void initAllocDesc(AllocDesc& ro_Desc);
	CORIUM_RUNTIME_API void setAllocationType(AllocDesc& ro_Desc, AllocationType v_Type);
	CORIUM_RUNTIME_API void setAllocationHandleType(AllocDesc& ro_Desc, AllocationHandleType v_Type);
	CORIUM_RUNTIME_API void setLocation(AllocDesc& ro_Desc, DeviceHandle& ro_Handle);

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(16) AccessDesc final {
		Location m_Loc;
		uint64_t flags;

		AccessDesc() = default;
		~AccessDesc() = default;

		AccessDesc(const AccessDesc&) = default;
		AccessDesc& operator=(const AccessDesc&) = default;

		AccessDesc(AccessDesc&&) noexcept = default;
		AccessDesc& operator=(AccessDesc&&) noexcept = default;
	};

	CORIUM_RUNTIME_API void initAccessDesc(AccessDesc& ro_Desc);
	CORIUM_RUNTIME_API void setAccessLocation(AccessDesc& ro_Desc, DeviceHandle& ro_Handle);
	CORIUM_RUNTIME_API void setAccessFlags(AccessDesc& ro_Desc, uint64_t v_Flags);
}
