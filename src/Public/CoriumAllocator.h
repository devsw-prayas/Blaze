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
	enum class CORIUM AllocationTrait : std::uint8_t {
		Scratch,
		Task,
		Executor,
		Persistent,
		Unknown
	};

	template <typename T, typename = void> struct CORIUM ResolveAllocation final {
		static constexpr AllocationTrait trait = AllocationTrait::Unknown;
	};

	struct CORIUM AllocationHeader final {};

	template <typename Derived> class CORIUM IAllocator {
		IAllocator() = default;
		~IAllocator() = default;

		IAllocator(const IAllocator&) = default;
		IAllocator& operator=(const IAllocator&) = default;

		IAllocator(IAllocator&&) noexcept = default;
		IAllocator& operator=(IAllocator&&) noexcept = default;

		template <typename T> AllocationHeader allocate() {
			AllocationHeader header;
		}
	};
} // namespace Corium::Memory::Allocators
