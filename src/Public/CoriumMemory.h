/*
* Copyright (c) 2025 StormWeaver
*
* This file is part of the Corium Multithreading API
*
* Licensed under the MIT License. You may obtain a copy of the License at
* https://opensource.org/licenses/MIT
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND...
*/
#pragma once
#include <Corium.h>

namespace Corium::Memory {
	using Bytes = size_t;

	struct CORIUM alignas(32) VirtualSegment final {
		void* m_Memory;
		Bytes v_TotalSize;
		Bytes v_CommittedSize;

		VirtualSegment(const VirtualSegment&) = default;
		VirtualSegment& operator=(const VirtualSegment&) = default;

		VirtualSegment(VirtualSegment&&) noexcept = default;
		VirtualSegment& operator=(VirtualSegment&&) noexcept = default;

		constexpr VirtualSegment(void* p_Memory = nullptr, Bytes v_TSize = 0, Bytes v_CSize = 0) :
			m_Memory(p_Memory), v_TotalSize(v_TSize), v_CommittedSize(v_CSize) {
		}

		constexpr bool isValid() const noexcept {
			return m_Memory != nullptr && v_TotalSize != 0;
		}

		~VirtualSegment() = default;
	};

	inline constexpr VirtualSegment INVALID_SEGMENT{};

	namespace Literals {
		constexpr Bytes CORIUM operator""_KB(unsigned long long v_KB) {
			return v_KB * 1000ULL;
		}

		constexpr Bytes CORIUM operator""_KiB(unsigned long long v_KiB) {
			return v_KiB * 1024ULL;
		}

		constexpr Bytes CORIUM operator""_MB(unsigned long long v_MB) {
			return v_MB * 1000ULL * 1000ULL;
		}

		constexpr Bytes CORIUM operator""_MiB(unsigned long long v_MiB) {
			return v_MiB * 1024ULL * 1024ULL;
		}

		constexpr Bytes CORIUM operator""_GB(unsigned long long v_GB) {
			return v_GB * 1000ULL * 1000ULL * 1000ULL;
		}

		constexpr Bytes CORIUM operator""_GiB(unsigned long long v_GiB) {
			return v_GiB * 1024ULL * 1024ULL * 1024ULL;
		}
	}

	using namespace Literals;
	constexpr Bytes PAGE_SIZE = 4_KiB;

	[[nodiscard]] CORIUM_FORCEINLINE constexpr Bytes alignToPage(unsigned long long v_Bytes) {
		return (v_Bytes + PAGE_SIZE - 1) / PAGE_SIZE * PAGE_SIZE;
	}

	constexpr Bytes KILO_BYTE = 1_KB;
	constexpr Bytes MEGA_BYTE = 1_MB;
	constexpr Bytes GIGA_BYTE = 1_GB;

	constexpr Bytes KIBI_BYTE = 1_KiB;
	constexpr Bytes MEBI_BYTE = 1_MiB;
	constexpr Bytes GIBI_BYTE = 1_GiB;

	enum class MemoryOperation : std::uint8_t {
		Reserve, Commit, Decommit, Free
	};

	enum class MemState : std::uint8_t {
		Reserved, Committed, Freed
	};

	class CORIUM VirtualMemory final {
	public:
		[[nodiscard]]
		static VirtualSegment virtualAlloc(
			VirtualSegment& segment,
			Bytes v_Size,
			MemoryOperation v_Operation
		);

		[[nodiscard]]
		static bool virtualFree(
			VirtualSegment& segment,
			Bytes v_Size,
			MemoryOperation v_Operation
		);

		[[nodiscard]]
		static bool lockMem(const VirtualSegment& segment);

		[[nodiscard]]
		static bool unlockMem(const VirtualSegment& segment);

		[[nodiscard]]
		static MemState queryPage(const VirtualSegment& segment, Bytes v_Offset);
	};

	namespace Internal {
		struct alignas(32) VARegion final {
			uint8_t* m_Base;
			Bytes    m_Size;

			constexpr VARegion() noexcept
				: m_Base(nullptr), m_Size(0) {
			}

			constexpr VARegion(uint8_t* p_Base, Bytes v_Size) noexcept
				: m_Base(p_Base), m_Size(v_Size) {
			}

			[[nodiscard]]
			constexpr bool isValid() const noexcept {
				return m_Base != nullptr && m_Size != 0;
			}

			VARegion(const VARegion&) = default;
			VARegion& operator=(const VARegion&) = default;

			VARegion(VARegion&&) noexcept = default;
			VARegion& operator=(VARegion&&) noexcept = default;

			~VARegion() = default;
		};

		inline constexpr VARegion INVALID_REGION{};

		class VARegionSlicer final {
		public:
			VARegionSlicer() : m_Cursor(nullptr), m_End(nullptr) {}

			explicit VARegionSlicer(const VirtualSegment& r_Segment) noexcept
				: m_Cursor(static_cast<uint8_t*>(r_Segment.m_Memory)),
				m_End(static_cast<uint8_t*>(r_Segment.m_Memory) + r_Segment.v_TotalSize) {
			}

			explicit VARegionSlicer(const VARegion& r_Region) noexcept
				: m_Cursor(r_Region.m_Base),
				m_End(r_Region.m_Base + r_Region.m_Size) {
			}

			~VARegionSlicer() = default;

			VARegionSlicer(VARegionSlicer&&) noexcept = default;
			VARegionSlicer& operator=(VARegionSlicer&&) noexcept = default;

			VARegionSlicer(const VARegionSlicer&) = delete;
			VARegionSlicer& operator=(const VARegionSlicer&) = delete;

			[[nodiscard]]
			VARegion slice(Bytes v_RequestedSize) noexcept {
				Bytes size = alignToPage(v_RequestedSize);

#if defined(CORIUM_DEBUG)
				if (m_Cursor + size > m_End) {
					Unreachable();
				}
#endif

				VARegion region{
					m_Cursor,
					size
				};

#if defined(CORIUM_DEBUG)
				// Structural invariants
				if ((reinterpret_cast<uintptr_t>(region.m_Base) & (PAGE_SIZE - 1)) != 0) {
					Unreachable();
				}
				if ((region.m_Size & (PAGE_SIZE - 1)) != 0) {
					Unreachable();
				}
#endif

				m_Cursor += size;
				return region;
			}

			[[nodiscard]]
			CORIUM_FORCEINLINE constexpr Bytes remaining() const noexcept {
				return static_cast<Bytes>(m_End - m_Cursor);
			}

		private:
			uint8_t* m_Cursor;
			uint8_t* m_End;
		};

		CORIUM_FORCEINLINE VirtualSegment segmentFromRegion(const VARegion& r_Region) noexcept {
#if defined(CORIUM_DEBUG)
			if (!r_Region.isValid()) {
				Unreachable();
			}
#endif

			VirtualSegment segment{};
			segment.m_Memory = r_Region.m_Base;
			segment.v_TotalSize = r_Region.m_Size;
			return segment;
		}

		CORIUM_FORCEINLINE bool lockGuard(const VARegion& r_Guard) noexcept {
			return VirtualMemory::lockMem(segmentFromRegion(r_Guard));
		}


	}

	template<typename T>
	struct CORIUM alignas(8) SharedPtr final {
	};

	template<typename T>
	struct CORIUM alignas(8) WeakPtr final {
	};

	template<typename T>
	struct CORIUM alignas(8) UniquePtr final {
	};
}
