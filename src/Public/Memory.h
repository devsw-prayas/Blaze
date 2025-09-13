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
#include "Corium.h"

namespace Corium::Memory {
	constexpr size_t CORIUM operator""_KB(unsigned long long v_KB) {
		return v_KB * 1000ULL;
	}

	constexpr size_t CORIUM operator""_KiB(unsigned long long v_KiB) {
		return v_KiB * 1024ULL;
	}

	constexpr size_t CORIUM operator""_MB(unsigned long long v_MB) {
		return v_MB * 1000ULL * 1000ULL;
	}

	constexpr size_t CORIUM operator""_MiB(unsigned long long v_MiB) {
		return v_MiB * 1024ULL * 1024ULL;
	}

	constexpr size_t CORIUM operator""_GB(unsigned long long v_GB) {
		return v_GB * 1000ULL * 1000ULL * 1000ULL;
	}

	constexpr size_t CORIUM operator""_GiB(unsigned long long v_GiB) {
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

	inline Memory CORIUM reserve(const size_t v_Size);

	inline bool CORIUM commit(Memory& ro_Memory, const size_t v_BlockSize, const size_t v_BlockOffset);

	inline Memory CORIUM allocate(const size_t v_Size);

	inline bool CORIUM decommit(Memory& ro_Memory, const size_t v_BlockSize, const size_t v_BlockOffset);

	inline bool CORIUM release(Memory& ro_Memory);

	inline void* CORIUM heapAlloc(size_t v_Bytes);

	template<typename T, typename ...Args>
	bool CORIUM emplaceHeap(void* p_Heap, Args&&...u_Args);

	template<typename T>
	inline bool CORIUM heapFree(void* p_Heap);
}