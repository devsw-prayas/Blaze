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
#include <Corium.h>

namespace Corium::Memory::Allocators {
	enum class CORIUM_RUNTIME_API AllocationTrait : std::uint8_t {
		Scratch,
		Task,
		Executor,
		Persistent,
		Unknown
	};

	template <typename T, typename = void> struct CORIUM_RUNTIME_API ResolveAllocation final {
		static constexpr AllocationTrait trait = AllocationTrait::Unknown;
	};

	struct alignas(64) CORIUM_RUNTIME_API AllocationHeader final {
		void* m_Block;
		size_t m_Size;
		size_t m_Alignment;
		AllocationTrait m_Trait;

		AllocationHeader(void* p_Block, size_t v_Size, size_t v_Alignment, AllocationTrait v_Trait) :
			m_Block(p_Block), m_Size(v_Size), m_Alignment(v_Alignment), m_Trait(v_Trait) {
		}

		~AllocationHeader() = default;
		AllocationHeader(const AllocationTrait&) = delete;
		AllocationHeader& operator=(const AllocationHeader&) = delete;

		AllocationHeader(AllocationHeader&&) noexcept = default;
		AllocationHeader& operator=(AllocationHeader&&) noexcept = default;

		template<typename T>
		T* as() const noexcept {
			return static_cast<T*>(m_Block);
		}
	};

	CORIUM_STATIC_ASSERT(std::is_standard_layout_v<AllocationHeader>,
						 "AllocationHeader must maintain standard layout");
	CORIUM_STATIC_ASSERT(std::is_trivially_copyable_v<AllocationHeader>,
						 "AllocationHeader must be trivially copyable");
	CORIUM_STATIC_ASSERT(std::is_trivially_move_assignable_v<AllocationHeader>,
						 "AllocationHeader must be trivially move assignable");

	template <typename D> 
	class CORIUM_RUNTIME_API IAllocator {
		using derived_ = D;
		IAllocator() = default;
		~IAllocator() = default;

		IAllocator(const IAllocator&) = default;
		IAllocator& operator=(const IAllocator&) = default;

		IAllocator(IAllocator&&) noexcept = default;
		IAllocator& operator=(IAllocator&&) noexcept = default;

		AllocationHeader allocate(size_t v_Size) {
			return static_cast<derived_*>(this)->allocate(v_Size);
		}

		bool deallocate(AllocationHeader& ro_Header) {
			return static_cast<derived_*>(this)->deallocate(ro_Header);
		}
	};
} // namespace Corium::Memory::Allocators
