#pragma once

#include "AtomicVariable.h"
#include "CoriumAllocator.h"
#include "CoriumAddrSpace.h"

namespace Corium::Memory::Allocators {
	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(64) BumpAllocatorBase {
		VirtualSegment m_Base;
		size_t m_Size = 0;
		CORIUM_ALIGNAS(8) Core::Atomic::AtomicValue64<size_t> m_Bump{ 0 };

		BumpAllocatorBase() = default;
		BumpAllocatorBase(const BumpAllocatorBase&) = delete;
		BumpAllocatorBase& operator=(const BumpAllocatorBase&) = delete;
		BumpAllocatorBase(BumpAllocatorBase&&) noexcept = delete;
		BumpAllocatorBase& operator=(BumpAllocatorBase&&) noexcept = delete;
		~BumpAllocatorBase() = default;

		void init(VirtualSegment& ro_Base) noexcept {
			m_Base = ro_Base;
			m_Size = m_Base.v_TotalSize;
			m_Bump.store(0, Core::Atomics::MemoryOrder::RELAXED);
		}

		CORIUM_NODISCARD_MSG("Cannot discard allocated block pointer")
			void* allocate(size_t v_Bytes, size_t v_Align) noexcept;

		void deallocate(void* /*p_Block*/, size_t /*v_Bytes*/) noexcept {}
	};

	struct CORIUM_RUNTIME_API TaskMetadataAllocator final : BumpAllocatorBase {};

	template<>
	struct CORIUM_RUNTIME_API ResolveAllocation<TaskMetadataAllocator> final {
		static constexpr AllocationTrait trait = AllocationTrait::Persistent;
	};

	struct CORIUM_RUNTIME_API TaskPayloadAllocator final : BumpAllocatorBase {};

	template<>
	struct CORIUM_RUNTIME_API ResolveAllocation<TaskPayloadAllocator> final {
		static constexpr AllocationTrait trait = AllocationTrait::Persistent;
	};

	struct CORIUM_RUNTIME_API ControlBlockAllocator final : BumpAllocatorBase {};

	template<>
	struct CORIUM_RUNTIME_API ResolveAllocation<ControlBlockAllocator> final {
		static constexpr AllocationTrait trait = AllocationTrait::Persistent;
	};

	struct CORIUM_RUNTIME_API ClosureAllocator final : BumpAllocatorBase {
		template<typename T, typename... Args>
		CORIUM_FORCEINLINE
			T* emplace(Args&&... args) noexcept {
			void* mem = allocate(sizeof(T), alignof(T));
			if (!mem) return nullptr;

			return ::new (mem) T(std::forward<Args>(args)...);
		}
	};

	template<>
	struct CORIUM_RUNTIME_API ResolveAllocation<ClosureAllocator> final {
		static constexpr AllocationTrait trait = AllocationTrait::Persistent;
	};



}
