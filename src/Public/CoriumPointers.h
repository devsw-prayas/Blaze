#pragma once
#include "Corium.h"
#include "CoriumMemoryHandler.h"

namespace Corium::Memory {
	template<typename T, typename A = Allocators::ControlBlockAllocator>
	struct alignas(16) UniquePtr final {
		using allocator_ = A;
	private:
		allocator_* m_Allocator = nullptr;
		T* m_Memory = nullptr;
		void alloc() {
		}

		void destroy() {
		}
	public:
		UniquePtr(allocator_* po_Allocator, T&& u_Object) {
			m_Allocator = po_Allocator;
			alloc();
		}

		UniquePtr(const UniquePtr&) = delete;
		UniquePtr& operator=(const UniquePtr&) = delete;

		UniquePtr(UniquePtr&& u_Other) noexcept {
			u_Other.m_Allocator = nullptr;
			u_Other->m_Memory = nullptr;

			this->m_Memory = u_Other.m_Memory;
			this->m_Allocator = u_Other.m_Allocator;
		}

		UniquePtr& operator=(UniquePtr&& u_Other) noexcept {
		}

		~UniquePtr() {
			destroy();
			m_Allocator = nullptr;
		}
	};

	template<typename T>
	struct CORIUM_RUNTIME_API alignas(16) SharedPtr final {
	};

	template<typename T>
	struct CORIUM_RUNTIME_API alignas(16) WeakPtr final {
	};
}
