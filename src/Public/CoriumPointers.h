#pragma once
#include "CoriumMemoryHandler.h"
#include "CoriumAtomics.h"

namespace Corium::Memory {
	using namespace Corium::Core::Atomics;
	// UniqueView - non-owning observer vended by UniquePtr

	template<typename T>
	struct alignas(16) UniqueView final {
	private:
		T* m_Ptr = nullptr;

		template<typename U>
		friend struct UniquePtr;

		explicit UniqueView(T* p_Ptr) noexcept : m_Ptr(p_Ptr) {}

	public:
		UniqueView() noexcept = default;

		T* operator->() const noexcept { return m_Ptr; }
		T& operator*()  const noexcept { return *m_Ptr; }
		T* get()        const noexcept { return m_Ptr; }

		explicit operator bool() const noexcept { return m_Ptr != nullptr; }

		operator UniqueView<const T>() const noexcept {
			return UniqueView<const T>(m_Ptr);
		}
	};

	// UniquePtr<T> - single-owner smart pointer backed by GeneralAllocator.
	// The allocator instance is passed at construction and stored alongside
	// the pointer so destroy() can return memory without a separate free list.

	template<typename T>
	struct alignas(16) UniquePtr final {
	private:
		Allocators::GeneralAllocator* m_Allocator = nullptr;
		T* m_Memory = nullptr;

		template<typename... Args>
		void alloc(Args&&... u_Params) {
			CORIUM_ASSERT(m_Allocator != nullptr);
			T* p_Mem = m_Allocator->emplace<T>(std::forward<Args>(u_Params)...);
			if (p_Mem) {
				m_Memory = p_Mem;
			} else {
				m_Memory = nullptr;
				CORIUM_ASSERT(false && "Allocation failed in UniquePtr");
			}
		}

		void destroy() noexcept {
			if (m_Memory) {
				CORIUM_ASSERT(m_Allocator != nullptr);
				m_Memory->~T();
				m_Allocator->deallocate(m_Memory, sizeof(T));
				m_Memory = nullptr;
			}
		}

	public:
		UniquePtr() noexcept = default;

		template<typename... Args>
		explicit UniquePtr(Allocators::GeneralAllocator* p_Allocator,
						   Args&&... u_Params) noexcept {
			m_Allocator = p_Allocator;
			alloc(std::forward<Args>(u_Params)...);
		}

		UniqueView<T> view() noexcept { return UniqueView<T>(m_Memory); }
		UniqueView<const T> view() const noexcept { return UniqueView<const T>(m_Memory); }

		UniquePtr(const UniquePtr&) = delete;
		UniquePtr& operator=(const UniquePtr&) = delete;

		UniquePtr(UniquePtr&& u_Other) noexcept
			: m_Allocator(u_Other.m_Allocator), m_Memory(u_Other.m_Memory) {
			u_Other.m_Allocator = nullptr;
			u_Other.m_Memory = nullptr;
		}

		UniquePtr& operator=(UniquePtr&& u_Other) noexcept {
			if (this == &u_Other) return *this;
			destroy();
			m_Allocator = u_Other.m_Allocator;
			m_Memory = u_Other.m_Memory;
			u_Other.m_Allocator = nullptr;
			u_Other.m_Memory = nullptr;
			return *this;
		}

		~UniquePtr() {
			destroy();
			m_Allocator = nullptr;
		}

		T* operator->() const noexcept { return m_Memory; }
		T& operator*()  const noexcept { return *m_Memory; }
		T* get()        const noexcept { return m_Memory; }

		explicit operator bool() const noexcept { return m_Memory != nullptr; }

		T* release() noexcept {
			T* p_Ret = m_Memory;
			m_Memory = nullptr;
			m_Allocator = nullptr;
			return p_Ret;
		}

		void reset() noexcept {
			destroy();
			m_Allocator = nullptr;
		}

		template<typename... Args>
		void reset(Allocators::GeneralAllocator* p_Allocator, Args&&... u_Params) {
			destroy();
			m_Allocator = p_Allocator;
			alloc(std::forward<Args>(u_Params)...);
		}
	};

	// Internal - SharedControlBlock
	//
	// Bump-allocated from ControlBlockAllocator (g_SmartPtrControlBlocks VA).
	// Never individually freed - lives for the process lifetime.
	// No type-erased deleter: SharedPtr<T> carries the GeneralAllocator* directly
	// and destroys the object itself when the count hits zero.

	namespace Internal {
		struct alignas(64) SharedControlBlock final {
			Core::Atomic::AtomicValue64<uint64_t> m_StrongCount;

		private:
			CORIUM_MAYBE_UNUSED const std::byte padding_[56] = {};
		public:
			explicit SharedControlBlock(uint64_t v_InitCount) noexcept
				: m_StrongCount(v_InitCount) {
			}

			SharedControlBlock(const SharedControlBlock&) = delete;
			SharedControlBlock& operator=(const SharedControlBlock&) = delete;
			SharedControlBlock(SharedControlBlock&&) = delete;
			SharedControlBlock& operator=(SharedControlBlock&&) = delete;

			// Increment strong count. Called on SharedPtr copy. Never fails.
			void addRef() noexcept {
				CORIUM_UNUSED(m_StrongCount.increment(1u, MemoryOrder::RELAXED));
			}

			// Decrement strong count. Returns true if this was the last reference.
			// atomicDecrement64 returns the new (post-decrement) value.
			bool release() noexcept {
				uint64_t v_NewCount = m_StrongCount.decrement(1u, MemoryOrder::ACQ_REL);
				return v_NewCount == 0u;
			}

			// CAS loop: try to increment strong count from a non-zero value.
			// Used by WeakPtr::lock() to atomically promote a weak reference.
			// Returns true on success (caller now holds a strong reference).
			bool tryAddRef() noexcept {
				uint64_t cur = m_StrongCount.load(MemoryOrder::ACQUIRE);
				while (cur != 0u) {
					uint64_t v_Prev = cur;
					// compareExchange updates cur on failure to the actual current value.
					// On success cur is unchanged (still == v_Prev).
					CORIUM_UNUSED(m_StrongCount.compareExchange(
						&cur, cur + 1u,
						MemoryOrder::ACQ_REL,
						MemoryOrder::ACQUIRE));
					if (cur == v_Prev) return true;
				}
				return false;
			}

			uint32_t useCount() const noexcept {
				return static_cast<uint32_t>(m_StrongCount.load(MemoryOrder::ACQUIRE));
			}
		};

		CORIUM_STATIC_ASSERT(sizeof(SharedControlBlock) <= 64,
							 "SharedControlBlock must fit in one cache line");
	} // namespace Internal

	// Forward declarations
	template<typename T> struct SharedPtr;
	template<typename T> struct WeakPtr;

	// SharedPtr<T> - shared-ownership smart pointer backed by GeneralAllocator.
	//
	// Each instance carries both the object pointer and the GeneralAllocator*
	// that owns it. All copies share the same control block (bump-allocated from
	// ControlBlockAllocator). When the last strong reference drops, the holder
	// calls ~T() and returns memory via the stored allocator directly - no
	// type-erased deleter needed.
	//
	// Construction:
	//   SharedPtr<T>::adopt(p_Object, p_ObjAlloc, p_CtrlAlloc)
	//     Takes ownership of a pre-constructed object.
	//
	//   SharedPtr<T>::make(p_ObjAlloc, p_CtrlAlloc, args...)
	//     Constructs the object via p_ObjAlloc and adopts it.

	template<typename T>
	struct alignas(16) SharedPtr final {
	private:
		Allocators::GeneralAllocator* m_Allocator = nullptr;
		T* m_Ptr = nullptr;
		Internal::SharedControlBlock* m_Control = nullptr;

		// Private constructor - used by adopt(), make(), copy, and WeakPtr::lock().
		SharedPtr(T* p_Ptr,
				  Internal::SharedControlBlock* p_Control,
				  Allocators::GeneralAllocator* p_Allocator) noexcept
			: m_Allocator(p_Allocator), m_Ptr(p_Ptr), m_Control(p_Control) {
		}

		void releaseRef() noexcept {
			if (m_Control && m_Control->release()) {
				CORIUM_ASSERT(m_Allocator != nullptr);
				m_Ptr->~T();
				m_Allocator->deallocate(m_Ptr, sizeof(T));
			}
			m_Allocator = nullptr;
			m_Ptr = nullptr;
			m_Control = nullptr;
		}

		template<typename U> friend struct SharedPtr;
		template<typename U> friend struct WeakPtr;

	public:
		SharedPtr() noexcept = default;

		// Take ownership of a pre-constructed object.
		// p_ObjAlloc  - GeneralAllocator that owns p_Object.
		// p_CtrlAlloc - ControlBlockAllocator for the ref-count block.
		static SharedPtr adopt(T* p_Object,
							   Allocators::GeneralAllocator* p_ObjAlloc,
							   Allocators::ControlBlockAllocator* p_CtrlAlloc) noexcept {
			CORIUM_ASSERT(p_Object && "Cannot adopt null object");
			CORIUM_ASSERT(p_ObjAlloc && "GeneralAllocator is null");
			CORIUM_ASSERT(p_CtrlAlloc && "ControlBlockAllocator is null");

			auto* p_Control = p_CtrlAlloc->emplace<Internal::SharedControlBlock>(1u);
			CORIUM_ASSERT(p_Control && "ControlBlockAllocator out of capacity");

			return SharedPtr(p_Object, p_Control, p_ObjAlloc);
		}

		// Construct and adopt in one call.
		template<typename... Args>
		static SharedPtr make(Allocators::GeneralAllocator* p_ObjAlloc,
							  Allocators::ControlBlockAllocator* p_CtrlAlloc,
							  Args&&... u_Args) noexcept {
			CORIUM_ASSERT(p_ObjAlloc && "GeneralAllocator is null");
			CORIUM_ASSERT(p_CtrlAlloc && "ControlBlockAllocator is null");

			T* p_Object = p_ObjAlloc->emplace<T>(std::forward<Args>(u_Args)...);
			if (!p_Object) return SharedPtr{};

			return adopt(p_Object, p_ObjAlloc, p_CtrlAlloc);
		}

		// Copy - increments strong count. Carries the same allocator pointer.
		SharedPtr(const SharedPtr& r_Other) noexcept
			: m_Allocator(r_Other.m_Allocator)
			, m_Ptr(r_Other.m_Ptr)
			, m_Control(r_Other.m_Control) {
			if (m_Control) m_Control->addRef();
		}

		SharedPtr& operator=(const SharedPtr& r_Other) noexcept {
			if (this == &r_Other) return *this;
			releaseRef();
			m_Allocator = r_Other.m_Allocator;
			m_Ptr = r_Other.m_Ptr;
			m_Control = r_Other.m_Control;
			if (m_Control) m_Control->addRef();
			return *this;
		}

		// Move - steals ownership, no ref count change.
		SharedPtr(SharedPtr&& u_Other) noexcept
			: m_Allocator(u_Other.m_Allocator)
			, m_Ptr(u_Other.m_Ptr)
			, m_Control(u_Other.m_Control) {
			u_Other.m_Allocator = nullptr;
			u_Other.m_Ptr = nullptr;
			u_Other.m_Control = nullptr;
		}

		SharedPtr& operator=(SharedPtr&& u_Other) noexcept {
			if (this == &u_Other) return *this;
			releaseRef();
			m_Allocator = u_Other.m_Allocator;
			m_Ptr = u_Other.m_Ptr;
			m_Control = u_Other.m_Control;
			u_Other.m_Allocator = nullptr;
			u_Other.m_Ptr = nullptr;
			u_Other.m_Control = nullptr;
			return *this;
		}

		~SharedPtr() { releaseRef(); }

		T* get()        const noexcept { return m_Ptr; }
		T& operator*()  const noexcept { CORIUM_ASSERT(m_Ptr); return *m_Ptr; }
		T* operator->() const noexcept { CORIUM_ASSERT(m_Ptr); return m_Ptr; }

		explicit operator bool() const noexcept { return m_Ptr != nullptr; }

		uint32_t useCount() const noexcept {
			return m_Control ? m_Control->useCount() : 0u;
		}

		WeakPtr<T> weak() const noexcept;

		void reset() noexcept { releaseRef(); }
	};

	// WeakPtr<T> - non-owning observer for SharedPtr<T>-managed objects.
	//
	// Carries the GeneralAllocator* from the originating SharedPtr so that
	// lock() can reconstruct a full SharedPtr<T> if the object is still alive.
	// No ref counting in the destructor - control block is bump-allocated and
	// lives for the process lifetime.

	template<typename T>
	struct alignas(16) WeakPtr final {
	private:
		Allocators::GeneralAllocator* m_Allocator = nullptr;
		T* m_Ptr = nullptr;
		Internal::SharedControlBlock* m_Control = nullptr;

		template<typename U> friend struct SharedPtr;
		template<typename U> friend struct WeakPtr;

	public:
		WeakPtr() noexcept = default;

		// Construct from SharedPtr - does NOT increment the strong count.
		explicit WeakPtr(const SharedPtr<T>& r_Shared) noexcept
			: m_Allocator(r_Shared.m_Allocator)
			, m_Ptr(r_Shared.m_Ptr)
			, m_Control(r_Shared.m_Control) {
		}

		WeakPtr(const WeakPtr& r_Other) noexcept
			: m_Allocator(r_Other.m_Allocator)
			, m_Ptr(r_Other.m_Ptr)
			, m_Control(r_Other.m_Control) {
		}

		WeakPtr& operator=(const WeakPtr& r_Other) noexcept {
			if (this == &r_Other) return *this;
			m_Allocator = r_Other.m_Allocator;
			m_Ptr = r_Other.m_Ptr;
			m_Control = r_Other.m_Control;
			return *this;
		}

		WeakPtr(WeakPtr&& u_Other) noexcept
			: m_Allocator(u_Other.m_Allocator)
			, m_Ptr(u_Other.m_Ptr)
			, m_Control(u_Other.m_Control) {
			u_Other.m_Allocator = nullptr;
			u_Other.m_Ptr = nullptr;
			u_Other.m_Control = nullptr;
		}

		WeakPtr& operator=(WeakPtr&& u_Other) noexcept {
			if (this == &u_Other) return *this;
			m_Allocator = u_Other.m_Allocator;
			m_Ptr = u_Other.m_Ptr;
			m_Control = u_Other.m_Control;
			u_Other.m_Allocator = nullptr;
			u_Other.m_Ptr = nullptr;
			u_Other.m_Control = nullptr;
			return *this;
		}

		// No destructor work - control block is bump-allocated, lives forever.
		~WeakPtr() = default;

		// Atomically promote to SharedPtr via CAS on the strong count.
		// Returns an empty SharedPtr if the object has been destroyed.
		SharedPtr<T> lock() const noexcept {
			if (!m_Control || !m_Control->tryAddRef()) return SharedPtr<T>{};
			return SharedPtr<T>(m_Ptr, m_Control, m_Allocator);
		}

		bool expired() const noexcept {
			return !m_Control || m_Control->useCount() == 0u;
		}

		explicit operator bool() const noexcept { return !expired(); }

		void reset() noexcept {
			m_Allocator = nullptr;
			m_Ptr = nullptr;
			m_Control = nullptr;
		}
	};

	// Deferred - defined after WeakPtr is complete.
	template<typename T>
	WeakPtr<T> SharedPtr<T>::weak() const noexcept {
		return WeakPtr<T>(*this);
	}
} 
