#pragma once

#include "AtomicVariable.h"
#include "CoriumAllocator.h"
#include "CoriumMemory.h"

namespace Corium::Memory::Allocators {
	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(64) BumpAllocator : IArenaAllocator<BumpAllocator> {
		Core::Atomic::AtomicValue64<size_t> m_Bump{ 0 };
		VirtualSegment* m_Base;
		size_t m_Size = 0;

		BumpAllocator() = default;
		BumpAllocator(const BumpAllocator&) = delete;
		BumpAllocator& operator=(const BumpAllocator&) = delete;
		BumpAllocator(BumpAllocator&&) noexcept = delete;
		BumpAllocator& operator=(BumpAllocator&&) noexcept = delete;
		~BumpAllocator() = default;

		void init(VirtualSegment * ro_Base) noexcept {
			m_Base = ro_Base;
			m_Size = m_Base->m_TotalSize;
			m_Bump.store(0, Core::Atomics::MemoryOrder::RELAXED);
		}

		CORIUM_NODISCARD_MSG("Cannot discard allocated block pointer")
			void* allocateImpl(size_t v_Bytes, size_t v_Align) noexcept;

		CORIUM_NODISCARD_MSG("Cannot discard allocated block pointer")
			void* allocateImpl(size_t v_Bytes) noexcept;

		void deallocateImpl() noexcept {
			// No-op
		}

		void deallocateImpl(void* p_Memory, size_t v_Size) noexcept {
			// No-op
		}
	};

	struct CORIUM_RUNTIME_API TaskMetadataAllocator final : BumpAllocator {};

	template<>
	struct CORIUM_RUNTIME_API ResolveAllocation<TaskMetadataAllocator> final {
		static constexpr AllocationTrait trait = AllocationTrait::Persistent;
	};

	struct CORIUM_RUNTIME_API TaskPayloadAllocator final : BumpAllocator {};

	template<>
	struct CORIUM_RUNTIME_API ResolveAllocation<TaskPayloadAllocator> final {
		static constexpr AllocationTrait trait = AllocationTrait::Persistent;
	};

	struct CORIUM_RUNTIME_API ControlBlockAllocator final : BumpAllocator {
		template<typename T, typename... Args>
		CORIUM_FORCEINLINE
			T* emplace(Args&&... args) noexcept {
			void* mem = allocateImpl(sizeof(T), alignof(T));
			if (!mem) return nullptr;

			return ::new (mem) T(std::forward<Args>(args)...);
		}
	};

	template<>
	struct CORIUM_RUNTIME_API ResolveAllocation<ControlBlockAllocator> final {
		static constexpr AllocationTrait trait = AllocationTrait::Persistent;
	};

	struct CORIUM_RUNTIME_API ClosureAllocator final : BumpAllocator {
		template<typename T, typename... Args>
		CORIUM_FORCEINLINE
			T* emplace(Args&&... args) noexcept {
			void* mem = allocateImpl(sizeof(T), alignof(T));
			if (!mem) return nullptr;

			return ::new (mem) T(std::forward<Args>(args)...);
		}
	};

	template<>
	struct CORIUM_RUNTIME_API ResolveAllocation<ClosureAllocator> final {
		static constexpr AllocationTrait trait = AllocationTrait::Persistent;
	};

	// Initialise all per-node allocator instances. Must be called after
	// Corium::Memory::Internal::init() has reserved the VA regions.
	CORIUM_RUNTIME_API void init();

	struct alignas(64) CORIUM_RUNTIME_API GeneralAllocator final : IArenaAllocator<GeneralAllocator>{
	private:
		struct alignas(16) BlockHeader final {
			size_t m_SizeAndFlags;   // lower bit = free flag
			BlockHeader* m_NextFree;
			BlockHeader* m_PrevFree;

			static constexpr size_t FREE_BIT = 1;

			size_t size() const {
				return m_SizeAndFlags & ~FREE_BIT;
			}

			bool isFree() const {
				return (m_SizeAndFlags & FREE_BIT) != 0;
			}

			void set(size_t size, bool free) {
				m_SizeAndFlags = size | (free ? FREE_BIT : 0);
			}
		};

		struct BlockFooter final {
			size_t m_Size;
		};

	private:
		VirtualSegment* m_Segment = nullptr;
		uint8_t* m_Base = nullptr;
		uint8_t* m_Cursor = nullptr;
		size_t m_Size = 0;
		BlockHeader* m_FreeList = nullptr;

	private:
		static CORIUM_FORCEINLINE uintptr_t alignUp(uintptr_t v, size_t a) {
			return (v + a - 1) & ~(a - 1);
		}

		static CORIUM_FORCEINLINE BlockFooter* footer(BlockHeader* h) {
			return reinterpret_cast<BlockFooter*>(
				reinterpret_cast<uint8_t*>(h) + h->size() - sizeof(BlockFooter));
		}

		static CORIUM_FORCEINLINE BlockHeader* next(BlockHeader* h) {
			return reinterpret_cast<BlockHeader*>(
				reinterpret_cast<uint8_t*>(h) + h->size());
		}

		static CORIUM_FORCEINLINE BlockHeader* prev(BlockHeader* h) {
			auto* f = reinterpret_cast<BlockFooter*>(
				reinterpret_cast<uint8_t*>(h) - sizeof(BlockFooter));
			return reinterpret_cast<BlockHeader*>(
				reinterpret_cast<uint8_t*>(h) - f->m_Size);
		}

		void insertFree(BlockHeader* b) {
			b->m_NextFree = m_FreeList;
			b->m_PrevFree = nullptr;

			if (m_FreeList)
				m_FreeList->m_PrevFree = b;

			m_FreeList = b;
		}

		void removeFree(BlockHeader* b) {
			if (b->m_PrevFree)
				b->m_PrevFree->m_NextFree = b->m_NextFree;
			else
				m_FreeList = b->m_NextFree;

			if (b->m_NextFree)
				b->m_NextFree->m_PrevFree = b->m_PrevFree;
		}

		BlockHeader* findFree(size_t size) {
			BlockHeader* cur = m_FreeList;
			while (cur) {
				if (cur->size() >= size)
					return cur;
				cur = cur->m_NextFree;
			}
			return nullptr;
		}

		void split(BlockHeader* b, size_t needed) {
			size_t remaining = b->size() - needed;

			constexpr size_t MIN_SPLIT = sizeof(BlockHeader) + sizeof(BlockFooter) + 32;

			if (remaining < MIN_SPLIT)
				return;

			b->set(needed, false);

			auto* newBlock = reinterpret_cast<BlockHeader*>(
				reinterpret_cast<uint8_t*>(b) + needed);

			newBlock->set(remaining, true);

			footer(b)->m_Size = b->size();
			footer(newBlock)->m_Size = newBlock->size();

			insertFree(newBlock);
		}

		BlockHeader* coalesce(BlockHeader* b) {
			// next
			auto* n = next(b);
			if (reinterpret_cast<uint8_t*>(n) < m_Cursor && n->isFree()) {
				removeFree(n);
				b->set(b->size() + n->size(), true);
			}

			// prev
			if (reinterpret_cast<uint8_t*>(b) > m_Base) {
				auto* p = prev(b);
				if (p->isFree()) {
					removeFree(p);
					p->set(p->size() + b->size(), true);
					b = p;
				}
			}

			footer(b)->m_Size = b->size();
			return b;
		}

	public:
		template<typename T, typename... Args>
		CORIUM_FORCEINLINE
		T* emplace(Args&&... args) noexcept {
			void* mem = allocateImpl(sizeof(T), alignof(T));
			if (!mem) return nullptr;

			return ::new (mem) T(std::forward<Args>(args)...);
		}

		void init(VirtualSegment* seg) {
			CORIUM_ASSERT(seg && seg->isValid());

			m_Segment = seg;
			m_Base = static_cast<uint8_t*>(seg->m_Memory);
			m_Cursor = m_Base;
			m_Size = seg->m_TotalSize;

			m_FreeList = nullptr;
		}

		void* allocateImpl(size_t size, size_t alignment) {
			if (!m_Segment) return nullptr;

			uintptr_t raw = reinterpret_cast<uintptr_t>(m_Cursor);
			uintptr_t aligned = alignUp(raw + sizeof(BlockHeader), alignment);

			auto* header = reinterpret_cast<BlockHeader*>(aligned - sizeof(BlockHeader));
			uint8_t* userPtr = reinterpret_cast<uint8_t*>(aligned);

			size_t total = (userPtr - reinterpret_cast<uint8_t*>(header))
						 + size
						 + sizeof(BlockFooter);

			uint8_t* end = reinterpret_cast<uint8_t*>(header) + total;

			if (end <= m_Base + m_Size) {
				size_t offset = static_cast<size_t>(end - m_Base);

				if (!VirtualMemory::commitPageIfNeeded(*m_Segment, offset))
					return nullptr;

				header->set(total, false);
				footer(header)->m_Size = total;

				m_Cursor = end;
				return userPtr;
			}

			auto* b = findFree(total);
			if (!b) return nullptr;

			removeFree(b);
			split(b, total);

			b->set(b->size(), false);
			return reinterpret_cast<uint8_t*>(b) + sizeof(BlockHeader);
		}

		void* allocateImpl(size_t size) {
			return allocateImpl(size, alignof(std::max_align_t));
		}

		void deallocateImpl(void* ptr, size_t) {
			if (!ptr) return;

			auto* b = reinterpret_cast<BlockHeader*>(static_cast<uint8_t*>(ptr) - sizeof(BlockHeader));
			b->set(b->size(), true);
			b = coalesce(b);

			uint8_t* end = reinterpret_cast<uint8_t*>(b) + b->size();

			// cursor rewind
			if (end == m_Cursor) {
				m_Cursor = reinterpret_cast<uint8_t*>(b);
				return;
			}

			insertFree(b);
		}

		void deallocateImpl() {
			m_Cursor = m_Base;
			m_FreeList = nullptr;
		}
	};

}
