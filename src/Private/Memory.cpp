#include "Corium.h"
#include "Memory.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#elif defined(__linux__)
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <sched.h>
#endif

namespace Corium::Memory {
	Memory CORIUM reserve(const size_t v_Size) {
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

	bool commit(Memory& ro_Memory, const size_t v_BlockSize, const size_t v_BlockOffset) {
		if (ro_Memory.m_Committed + v_BlockSize > ro_Memory.m_Size || !ro_Memory.isValid()) return false;
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

	Memory allocate(const size_t v_Size) {
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

	bool decommit(Memory& ro_Memory, const size_t v_BlockSize, const size_t v_BlockOffset) {
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

	bool release(Memory& ro_Memory) {
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