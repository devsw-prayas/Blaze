#pragma once
#include "Blaze.h"

namespace Blaze::Memory {
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
	class BLAZE IAllocator {
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