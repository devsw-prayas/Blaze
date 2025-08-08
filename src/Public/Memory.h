#pragma once
#include "Blaze.h"

namespace Blaze::Memory {
	constexpr size_t BLAZE operator""_KB(unsigned long long v_KB) {
		return v_KB * 1000ULL;
	}

	constexpr size_t BLAZE operator""_KiB(unsigned long long v_KiB) {
		return v_KiB * 1024ULL;
	}

	constexpr size_t BLAZE operator""_MB(unsigned long long v_MB) {
		return v_MB * 1000ULL * 1000ULL;
	}

	constexpr size_t BLAZE operator""_MiB(unsigned long long v_MiB) {
		return v_MiB * 1024ULL * 1024ULL;
	}

	constexpr size_t BLAZE operator""_GB(unsigned long long v_GB) {
		return v_GB * 1000ULL * 1000ULL * 1000ULL;
	}

	constexpr size_t BLAZE operator""_GiB(unsigned long long v_GiB) {
		return v_GiB * 1024ULL * 1024ULL * 1024ULL;
	}

	constexpr size_t PAGE_FILE = 4_KiB;

	constexpr size_t alignToPage(unsigned long long v_Bytes) {
		return (v_Bytes + PAGE_FILE - 1) / PAGE_FILE * PAGE_FILE;
	}

	constexpr size_t KILO_BYTE = 1_KB;
	constexpr size_t MEGA_BYTE = 1_MB;
	constexpr size_t GIGA_BYTE = 1_GB;

	constexpr size_t KIBI_BYTE = 1_KiB;
	constexpr size_t MEBI_BYTE = 1_MiB;
	constexpr size_t GIBI_BYTE = 1_GiB;

	// [Size in bytes]: 16
	struct Memory final {
		void* m_Ptr;
		size_t m_Size;
		size_t m_Committed;

		[[nodiscard]] bool isValid() const {
			return m_Ptr != nullptr;
		}
	};

	inline Memory BLAZE reserve(const size_t v_Size) {
		Memory memory;
		memory.m_Ptr = nullptr;
		memory.m_Size = 0;
		memory.m_Committed = 0;
#if defined(_WIN32)
		LPVOID mem = VirtualAlloc(nullptr, v_Size, MEM_RESERVE, PAGE_NOACCESS);
		if (!mem) return memory;
		memory.m_Ptr = mem;
		memory.m_Size = v_Size;
		return memory;
#elif defined(__linux__)
		void* mem = mmap(nullptr, v_Size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (mem == MAP_FAILED) return memory;
		memory.m_Ptr = mem;
		memory.m_Size = v_Size;
		return memory;
#else 
		return memory;
#endif
	}

	inline bool commit(Memory& v_Memory, const size_t v_BlockSize, const size_t v_BlockOffset) {
		if (v_Memory.m_Committed + v_BlockSize > v_Memory.m_Size) return false;
#if defined(_WIN32)
		LPVOID mem = VirtualAlloc(v_Memory.m_Ptr + v_BlockOffset, v_BlockSize, MEM_COMMIT, PAGE_READWRITE);
		if (!mem) return false;
		v_Memory.m_Committed += v_BlockSize;
		return true;
#elif defined (__linux__)
		int mem = mprotect(v_Memory.m_Ptr + v_BlockOffset, v_BlockSize, PROT_READ | PROT_WRITE);
		if (mem == -1) return false;
		v_Memory.m_Committed += v_BlockSize;
		return true;
#else
		return false;
#endif
	}

	inline Memory allocate(const size_t v_Size) {
		Memory memory;
		memory.m_Ptr = nullptr;
		memory.m_Size = 0;
		memory.m_Committed = 0;
#if defined(_WIN32)
		LPVOID mem = VirtualAlloc(nullptr, v_Size, MEM_COMMIT, PAGE_READWRITE);
		if (!mem) return memory;
		memory.m_Ptr = mem;
		memory.m_Size = v_Size;
		memory.m_Committed = v_Size;
		return memory;
#elif defined(__linux__)
		void* mem = mmap(nullptr, v_Size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (mem == MAP_FAILED) return memory;
		memory.m_Ptr = mem;
		memory.m_Size = v_Size;
		memory.m_Committed = v_Size;
		return memory;
#else
		return memory;
#endif
	}
}