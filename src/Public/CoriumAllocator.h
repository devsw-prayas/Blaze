#pragma once
#include <Corium.h>
#include <atomic>

namespace Corium::Memory::Allocators {
	enum class CORIUM AllocationTrait : std::uint8_t {
		Scratch, Task, Executor, Persistent, Unknown
	};
	std::atomic<>
	template<typename T, typename = void>
	struct CORIUM ResolveAllocation final {
		static constexpr AllocationTrait trait = AllocationTrait::Unknown;
	};

	struct CORIUM AllocationHeader final {

	};

	template<typename Derived>
	class CORIUM IAllocator {
		IAllocator() = default;
		~IAllocator() = default;

		IAllocator(const IAllocator&) = default;
		IAllocator& operator=(const IAllocator&) = default;

		IAllocator(IAllocator&&) noexcept = default;
		IAllocator& operator=(IAllocator&&) noexcept = default;

		AllocationHeader allocate(T*)
	};
}
