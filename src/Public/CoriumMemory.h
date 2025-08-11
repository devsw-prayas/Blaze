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
#include "Corium.h"

namespace Corium::Memory {
	template<typename T>
	class SharedPointer {
		//STUB TODO
	};

	template<typename T>
	class UniquePointer {
		//STUB TODO
	};

	template<typename T>
	class WeakPointer{
		//STUB TODO
	};

	template<typename T>
	class HazardPointer {
		//STUB TODO
	};

	template<typename T, typename Derived>
	class CORIUM IAllocator {
		static_assert(std::is_base_of_v<IAllocator, Derived>, "Derived is not a subclass of IAllocator");
	public:
		T* allocate(size_t v_size) noexcept{
			return static_cast<Derived*>(this)->allocate(v_size);
		}

		void deallocate(T* p_memory) noexcept{
			static_cast<Derived*>(this)->deallocate(p_memory);
		}

		template<typename...Args>
		void construct(T* p_block, Args&&...args) noexcept{
			static_cast<Derived*>(this)->template construct<Args...>(p_block, std::forward<Args>(args)...);
		}

		void destroy(T* p_block) noexcept{
			static_cast<Derived*>(this)->destroy(p_block);
		}

		void* rawAllocate(size_t v_size) noexcept{
			return static_cast<Derived*>(this)->rawAllocate(v_size);
		}

		void rawDeallocate(void* p_memory) noexcept{
			static_cast<Derived*>(this)->rawDeallocate(p_memory);
		}
	};
}