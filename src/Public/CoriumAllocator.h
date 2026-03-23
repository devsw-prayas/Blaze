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
			m_Block(p_Block), m_Size(v_Size), m_Alignment(v_Alignment), m_Trait(v_Trait) {}

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

	template<bool UseHeader, bool UseType, typename D, typename U = void,
		typename T = std::conditional_t<UseHeader, AllocationHeader, std::conditional_t<UseType, U, void>>>
	class IAllocator {
		CORIUM_STATIC_ASSERT(!(UseHeader&& UseType), "Invalid allocator configuration");
		CORIUM_STATIC_ASSERT(!std::is_void_v<U> || !UseType, "Cannot set typed allocator to use void");
		using derived_ = D;
		using allocType_ = T;

	public:
		// Allocator Raw call with aligment
		allocType_* allocate(size_t v_BlockSize, size_t v_Alignment) requires (!UseHeader && !UseType) {
			return static_cast<derived_*>(this)->allocateImpl(v_BlockSize, v_Alignment);
		}

		// Allocator Raw call
		allocType_* allocate(size_t v_BlockSize) requires (!UseHeader && !UseType) {
			return static_cast<derived_*>(this)->allocateImpl(v_BlockSize);
		}

		// Typed array call
		allocType_* allocate(size_t count = 1) requires (!UseHeader && UseType) {
			return static_cast<derived_*>(this)->allocateImpl(count);
		}

		// Header based allocator call
		allocType_ allocate(size_t v_Size, size_t v_Align) requires (UseHeader && !UseType) {
			return static_cast<derived_*>(this)->allocateImpl(v_Size, v_Align);
		}

		// Single object
		template<typename... Args>
		allocType_* emplace(Args&&... v_Args) requires (!UseHeader && UseType) {
			allocType_* ptr = static_cast<derived_*>(this)->allocateImpl(1);
			if (!ptr) return nullptr;
			return ::new (ptr) allocType_(std::forward<Args>(v_Args)...);
		}

		// Array
		template<typename... Args>
		allocType_* emplace(size_t v_Count, Args&&... v_Args) requires (!UseHeader && UseType) {
			allocType_* ptr = static_cast<derived_*>(this)->allocateImpl(v_Count);
			if (!ptr) return nullptr;
			for (size_t i = 0; i < v_Count; ++i)
				::new (ptr + i) allocType_(std::forward<Args>(v_Args)...);
			return ptr;
		}

		// Raw bump: no args
		void deallocate() requires (!UseHeader && !UseType) {
			static_cast<derived_*>(this)->deallocateImpl();
		}

		// Raw ptr: freelist or discard — derived decides
		void deallocate(void* p_Ptr, size_t v_Size) requires (!UseHeader && !UseType) {
			static_cast<derived_*>(this)->deallocateImpl(p_Ptr, v_Size);
		}

		// Typed: ptr + count
		void deallocate(allocType_* v_Ptr, size_t v_Count) requires (!UseHeader && UseType) {
			static_cast<derived_*>(this)->deallocateImpl(v_Ptr, v_Count);
		}

		// Header-driven: full metadata, derived owns the decision
		void deallocate(std::conditional_t<UseHeader, allocType_, AllocationHeader>&& v_Header) 
		requires (UseHeader && !UseType) {
			static_cast<derived_*>(this)->deallocateImpl(std::move(v_Header));
		}

		IAllocator() = default;

		IAllocator(const IAllocator&) = default;
		IAllocator& operator=(const IAllocator&) = default;

		IAllocator(IAllocator&&) noexcept = default;
		IAllocator& operator=(IAllocator&&) noexcept = default;

		~IAllocator() = default;
	};

	template<typename D>
	concept ArenaImpl = requires(D & d, size_t n, size_t a, void* p) {
		{ d.allocateImpl(n, a) } -> std::same_as<void*>;
		{ d.allocateImpl(n) } -> std::same_as<void*>;
		{ d.deallocateImpl() } -> std::same_as<void>;
		{ d.deallocateImpl(p, n) } -> std::same_as<void>;
	};

	template<typename D, typename T>
	concept TypedImpl = requires(D & d, T * p, size_t n) {
		{ d.allocateImpl(n) } -> std::same_as<T*>;
		{ d.deallocateImpl(p, n) } -> std::same_as<void>;
	};

	template<typename D>
	concept HeaderImpl = requires(D & d, size_t n, size_t a, AllocationHeader h) {
		{ d.allocateImpl(n, a) } -> std::same_as<AllocationHeader>;
		{ d.deallocateImpl(std::move(h)) } -> std::same_as<void>;
	};

	template<typename D>
	class IArenaAllocator : public IAllocator<false, false, D> {};

	template<typename D, typename T> requires TypedImpl<D, T>
	class ITypedAllocator : public IAllocator<false, true, D, T> {};

	template<typename D> requires HeaderImpl<D>
	class IHeaderAllocator : public IAllocator<true, false, D> {};
}
