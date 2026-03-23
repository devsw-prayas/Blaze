#include "Corium.h"
#include "EngineAllocators.h"

namespace Corium::Memory::Allocators {
	void* BumpAllocator::allocateImpl(size_t v_Bytes, size_t v_Align) noexcept {
		if (!m_Base->isValid() || v_Bytes == 0 || v_Align == 0) return nullptr;

		CORIUM_ASSERT((v_Align & (v_Align - 1)) == 0);

		size_t current = m_Bump.load(Core::Atomics::MemoryOrder::RELAXED);

		for (;;) {
			size_t aligned = alignUp(current, v_Align);
			size_t next = aligned + v_Bytes;

			if (next > m_Size) return nullptr;

			if (m_Bump.compareExchange(
				&current,
				next,
				Core::Atomics::MemoryOrder::RELEASE,
				Core::Atomics::MemoryOrder::RELAXED)) {
				if (Memory::VirtualMemory::commitPageIfNeeded(*m_Base, next)) {
					m_Base->v_CommittedSize = alignUp(next, PAGE_SIZE);
					return static_cast<std::byte*>(m_Base->m_Memory) + aligned;
				}
				return nullptr;
			}
		}
	}

	void* BumpAllocator::allocateImpl(size_t v_Bytes) noexcept {
		return allocateImpl(v_Bytes, 0);
	}
}
