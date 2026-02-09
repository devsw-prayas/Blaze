#include "Corium.h"

#include <CoriumMemory.h>
#define ALLOW_SYSCALL
#include <CoriumSyscalls.h>

namespace Corium::Memory {
	VirtualSegment VirtualMemory::virtualAlloc(VirtualSegment& segment, Bytes v_Size, MemoryOperation v_Operation) {
#if defined(CORIYM_DEBUG)
		CORIUM_ASSERT(v_Size > 0);
#endif
		v_Size = alignToPage(v_Size);
#if defined(_WIN32)											   
		switch (v_Operation) {
		case MemoryOperation::Reserve:
		{
			if (segment.isValid())return INVALID_SEGMENT;
			void* p = VirtualAlloc(nullptr, v_Size, MEM_RESERVE, PAGE_NOACCESS);
			return p ? VirtualSegment{ p, v_Size, 0 } : INVALID_SEGMENT;
		}
		case MemoryOperation::Commit:
		{
			if (!segment.isValid())return INVALID_SEGMENT;
			if (segment.v_CommittedSize + v_Size > segment.v_TotalSize)return INVALID_SEGMENT;

			void* p = VirtualAlloc(static_cast<std::byte*>(segment.m_Memory) + segment.v_CommittedSize, v_Size, MEM_COMMIT, PAGE_READWRITE);
			if (!p)return INVALID_SEGMENT;
			segment.v_CommittedSize += v_Size;
			return segment;
		}
		default:return INVALID_SEGMENT;
		}

#elif defined(__linux__)

		switch (v_Operation) {
		case MemoryOperation::Reserve:
		{
			if (segment.isValid())return INVALID_SEGMENT;
			void* p = mmap(nullptr, v_Size, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
			if (p == MAP_FAILED)return INVALID_SEGMENT;
			return VirtualSegment{ p, v_Size, 0 };
		}

		case MemoryOperation::Commit:
		{
			if (!segment.isValid())return INVALID_SEGMENT;
			if (segment.v_CommittedSize + v_Size > segment.v_TotalSize)return INVALID_SEGMENT;

			void* p = static_cast<std::byte*>(segment.m_Memory) + segment.v_CommittedSize;
			if (mprotect(p, v_Size, PROT_READ | PROT_WRITE) == -1)return INVALID_SEGMENT;
			segment.v_CommittedSize += v_Size;
			return segment;
		}
		default:return INVALID_SEGMENT;
		}

#else
		CORIUM_UNREACHABLE();
#endif
	}

	bool VirtualMemory::virtualFree(
	VirtualSegment& segment,
	Bytes v_Size,
	MemoryOperation v_Operation
	) {
		if (!segment.isValid()) return false;
#if defined(CORIYM_DEBUG)
		CORIUM_ASSERT(v_Size > 0);
#endif
		v_Size = alignToPage(v_Size);

#if defined(_WIN32)

		switch (v_Operation) {
		case MemoryOperation::Decommit:
		{
			if (v_Size > segment.v_CommittedSize)
				return false;

			void* p = static_cast<std::byte*>(segment.m_Memory)
				+ (segment.v_CommittedSize - v_Size);

			if (!VirtualFree(p, v_Size, MEM_DECOMMIT))
				return false;

			segment.v_CommittedSize -= v_Size;
			return true;
		}

		case MemoryOperation::Free:
		{
			bool ok = VirtualFree(segment.m_Memory, 0, MEM_RELEASE);
			segment = INVALID_SEGMENT;
			return ok;
		}

		default:
			return false;
		}

#elif defined(__linux__)

		switch (v_Operation) {
		case MemoryOperation::Decommit:
		{
			if (v_Size > segment.v_CommittedSize)
				return false;

			void* p = static_cast<std::byte*>(segment.m_Memory)
				+ (segment.v_CommittedSize - v_Size);

			if (mprotect(p, v_Size, PROT_NONE) == -1)
				return false;

			madvise(p, v_Size, MADV_DONTNEED);

			segment.v_CommittedSize -= v_Size;
			return true;
		}

		case MemoryOperation::Free:
		{
			bool ok = munmap(segment.m_Memory, segment.v_TotalSize) != -1;
			segment = INVALID_SEGMENT;
			return ok;
		}

		default:
			return false;
		}

#else
		CORIUM_UNREACHABLE();
#endif
	}

	bool VirtualMemory::lockMem(const VirtualSegment& segment) {
		if (!segment.isValid() || segment.v_CommittedSize == 0)
			return false;

#if defined(_WIN32)
		return VirtualLock(segment.m_Memory, segment.v_CommittedSize);

#elif defined(__linux__)
		return mlock(segment.m_Memory, segment.v_CommittedSize) == 0;

#else
		CORIUM_UNREACHABLE();
#endif
	}

	bool VirtualMemory::unlockMem(const VirtualSegment& segment) {
		if (!segment.isValid() || segment.v_CommittedSize == 0)
			return false;

#if defined(_WIN32)
		return VirtualUnlock(segment.m_Memory, segment.v_CommittedSize);

#elif defined(__linux__)
		return munlock(segment.m_Memory, segment.v_CommittedSize) == 0;

#else
		CORIUM_UNREACHABLE();
#endif
	}

	MemState VirtualMemory::queryPage(const VirtualSegment& segment, Bytes v_Offset) {
#if defined(CORIUM_DEBUG)
		CORIUM_ASSERT(v_Offset < segment.v_TotalSize);
#endif
#if defined (_WIN32)
		MEMORY_BASIC_INFORMATION memInfo;
		void* offsetMem = static_cast<std::byte*>(segment.m_Memory) + v_Offset;
		VirtualQuery(offsetMem, &memInfo, sizeof(MEMORY_BASIC_INFORMATION));
		switch (memInfo.State) {
		case MEM_RESERVE:
			return MemState::Reserved;
		case MEM_COMMIT:
			return MemState::Committed;
		case MEM_FREE:
			return MemState::Freed;
		default:
			CORIUM_UNREACHABLE();
		}
#elif defined(__linux__)
#if defined(CORIUM_DEBUG)
		CORIUM_UNREACHABLE(); // TODO: /proc/self/maps + mincore
#else
		return MemState::Freed;
#endif
#else
		CORIUM_UNREACHABLE();
#endif
	}
}