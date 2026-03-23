#pragma once

#include "AtomicVariable.h"
#include "CoriumAllocator.h"
#include "CoriumAddrSpace.h"
#include "CoriumMemory.h"

namespace Corium::Memory::Allocators {
	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(64) BumpAllocator : IArenaAllocator<BumpAllocator> {
		VirtualSegment* m_Base;
		size_t m_Size = 0;
		CORIUM_ALIGNAS(8) Core::Atomic::AtomicValue64<size_t> m_Bump{ 0 };

		BumpAllocator() = default;
		BumpAllocator(const BumpAllocator&) = delete;
		BumpAllocator& operator=(const BumpAllocator&) = delete;
		BumpAllocator(BumpAllocator&&) noexcept = delete;
		BumpAllocator& operator=(BumpAllocator&&) noexcept = delete;
		~BumpAllocator() = default;

		void init(VirtualSegment * ro_Base) noexcept {
			m_Base = ro_Base;
			m_Size = m_Base->v_TotalSize;
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

	struct CORIUM_RUNTIME_API ControlBlockAllocator final : BumpAllocator {};

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
}
