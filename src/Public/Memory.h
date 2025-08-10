/*
* Copyright (c) 2025 StormWeaver
*
* This file is part of the Blaze Multithreading API
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

	template<typename T>
	class MemoryRange final {
		static constexpr size_t STRIDE = sizeof(T);
		std::byte* m_Current;
		Memory m_Block;
		size_t m_Offset;

	public:
		MemoryRange(Memory v_Memory) : m_Block(v_Memory) {
			if (!m_Block.isValid()) m_Current = nullptr;
			else m_Current = reinterpret_cast<std::byte*>(m_Block.m_Ptr);
			m_Offset = 0;
		}

		T* operator*() {
			return reinterpret_cast<T*>(m_Current);
		}

		MemoryRange* next() {
			m_Current += STRIDE;
			++m_Offset;
			return this;
		}

		T* operator[](size_t v_Idx) {
			return reinterpret_cast<T*>(reinterpret_cast<std::byte*>(m_Block.m_Ptr) + v_Idx * STRIDE);
		}

		bool equals(MemoryRange& other) const {
			return (m_Current == other.m_Current && m_Offset == other.m_Offset);
		}

		[[nodiscard]] size_t byteOffset() const {
			return m_Offset * STRIDE;
		}

		[[nodiscard]] size_t offset() const {
			return m_Offset;
		}

		T* begin() {
			return reinterpret_cast<T*>(m_Block.m_Ptr);
		}

		T* end() {
			return reinterpret_cast<T*>(reinterpret_cast<std::byte*>(m_Block.m_Ptr) + m_Block.m_Size);
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

	inline bool commit(Memory& ro_Memory, const size_t v_BlockSize, const size_t v_BlockOffset) {
		if (ro_Memory.m_Committed + v_BlockSize > ro_Memory.m_Size || !ro_Memory.isValid()s) return false;
		auto pByte = reinterpret_cast<std::byte*>(ro_Memory.m_Ptr);
#if defined(_WIN32)
		LPVOID mem = VirtualAlloc(reinterpret_cast<void*>(pByte + v_BlockOffset), v_BlockSize, MEM_COMMIT, PAGE_READWRITE);
		if (!mem) return false;
		ro_Memory.m_Committed += v_BlockSize;
		return true;
#elif defined (__linux__)
		int mem = mprotect(reinterpret_cast<void*>(pByte + v_BlockOffset), v_BlockSize, PROT_READ | PROT_WRITE);
		if (mem == -1) return false;
		ro_Memory.m_Committed += v_BlockSize;
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

	inline bool decommit(Memory& ro_Memory, const size_t v_BlockSize, const size_t v_BlockOffset) {
		if (ro_Memory.m_Committed < v_BlockSize || !ro_Memory.isValid()) return false;
		auto pByte = reinterpret_cast<std::byte*>(ro_Memory.m_Ptr);
#if defined(_WIN32)
		if (!VirtualFree(reinterpret_cast<void*>(pByte + v_BlockOffset), v_BlockSize, MEM_DECOMMIT)) return false;
		ro_Memory.m_Committed -= v_BlockSize;
		return true;
#elif defined (__linux__)
		if (mprotect(reinterpret_cast<void*>(pByte + v_BlockOffset), v_BlockSize, PROT_NONE) == -1) return false;
		if (madvise(reinterpret_cast<void*>(pByte + v_BlockOffset), v_BlockSize, MADV_DONTNEED) == -1) return false;
		ro_Memory.m_Committed -= v_BlockSize;
		return true;
#else
		return false;
#endif
	}

	inline bool release(Memory& ro_Memory) {
		if (!ro_Memory.isValid()) return false;
#if defined(_WIN32)
		if (VirtualFree(ro_Memory.m_Ptr, 0, MEM_RELEASE)) {
			ro_Memory.m_Ptr = nullptr;
			ro_Memory.m_Committed = 0;
			ro_Memory.m_Size = 0;
			return true;
		}
		return false;
#elif defined(__linux__)
		if (munmap(ro_Memory.m_Ptr, ro_Memory.m_Size) != -1) {
			ro_Memory.m_Ptr = nullptr;
			ro_Memory.m_Committed = 0;
			ro_Memory.m_Size = 0;
			return true;
		}
		return false;
#else
		return false;
#endif
	}
}