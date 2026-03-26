#pragma once

#include "Corium.h"

#ifdef CORIUM_CUDA_AVAILABLE
namespace Corium::Cuda {
	using Bytes = size_t;

	struct CORIUM_RUNTIME_API alignas(32) DeviceSegment final {
		void* m_DevicePtr;
		Bytes m_TotalSize;
		Bytes m_CommittedSize;

		DeviceSegment(const DeviceSegment&) = default;
		DeviceSegment& operator=(const DeviceSegment&) = default;

		DeviceSegment(DeviceSegment&&) noexcept = default;
		DeviceSegment& operator=(DeviceSegment&&) noexcept = default;

		constexpr DeviceSegment(void* p_Memory = nullptr, Bytes v_TSize = 0, Bytes v_CSize = 0) :
			m_DevicePtr(p_Memory), m_TotalSize(v_TSize), m_CommittedSize(v_CSize) {
		}

		constexpr bool isValid() const noexcept {
			return m_DevicePtr != nullptr && m_TotalSize != 0;
		}

		~DeviceSegment() = default;
	};

	inline constexpr DeviceSegment INVALID_DEVICE_SEGMENT{};

	struct CORIUM_RUNTIME_API alignas(32) PinnedSegment final {
		void* m_HostPtr;
		Bytes m_TotalSize;

		PinnedSegment(const PinnedSegment&) = default;
		PinnedSegment& operator=(const PinnedSegment&) = default;

		PinnedSegment(PinnedSegment&&) noexcept = default;
		PinnedSegment& operator=(PinnedSegment&&) noexcept = default;

		constexpr PinnedSegment(void* p_Memory = nullptr, Bytes v_TSize = 0) :
			m_HostPtr(p_Memory), m_TotalSize(v_TSize) {
		}

		constexpr bool isValid() const noexcept {
			return m_HostPtr != nullptr && m_TotalSize != 0;
		}

		~PinnedSegment() = default;
	};

	inline constexpr PinnedSegment INVALID_PINNED_SEGMENT{};

	class CORIUM_RUNTIME_API DeviceMemory final {
		static DeviceSegment deviceAlloc(DeviceSegment& ro_DevSegment, Bytes v_Size);
		static bool deviceFree(DeviceSegment& ro_DevSegment);

		static PinnedSegment pinnedAlloc(PinnedSegment& ro_PinSegment, Bytes v_Size);
		static bool pinnedFree(PinnedSegment& ro_PinSegment);

		static bool copyToDevice(DeviceSegment& ro_DevDst, const PinnedSegment& ro_PinSrc, Bytes v_Size, void* po_CudaStream);
		static bool copyToHost(PinnedSegment& roPinDst, const DeviceSegment& ro_DevSrc, Bytes v_Size, void* po_CudaStream);

		static bool deviceMemset(DeviceSegment& ro_DevSegment, int v_Val, Bytes v_Size, void* po_CudaStream);

		static Bytes queryFreeMemory();
	};
}
#endif
