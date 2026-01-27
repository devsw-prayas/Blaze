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

		VirtualSegment(void* p_Memory = nullptr, Bytes v_TSize = 0, Bytes v_CSize = 0) :
			m_Memory(p_Memory), v_TotalSize(v_TSize), v_CommittedSize(v_CSize) {}

		constexpr bool isValid() const noexcept {
			return m_Memory != nullptr && v_TotalSize != 0;
		}

		~VirtualSegment() = default;
	};

	static constexpr CORIUM VirtualSegment INVALID_SEGMENT{};

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
	constexpr Bytes PAGE_FILE = 4_KiB;

	[[nodiscard]] ForceInline constexpr Bytes alignToPage(unsigned long long v_Bytes) {
		return (v_Bytes + PAGE_FILE - 1) / PAGE_FILE * PAGE_FILE;
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

	class CORIUM VirtualMemory final {
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
	};																	

	template<typename T>
	struct SharedPtr final {
		
	};

	template<typename T>
	struct WeakPtr final {
		
	};

	template<typename T>
	struct UniquePtr final{
		
	};



}
