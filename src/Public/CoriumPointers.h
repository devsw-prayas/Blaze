#pragma once
#include "Corium.h"
#include "CoriumMemoryHandler.h"

namespace Corium::Memory {

    template<typename T>
    struct alignas(16) UniqueView final {
    private:
        T* m_Ptr = nullptr;

        template<typename U, typename A>
        friend struct UniquePtr;

        explicit UniqueView(T* p_Ptr) noexcept : m_Ptr(p_Ptr) {}

    public:
        UniqueView() noexcept = default;

        T* operator->() const noexcept {
            return m_Ptr;
        }

        T& operator*() const noexcept {
            return *m_Ptr;
        }

        T* get() const noexcept {
            return m_Ptr;
        }

        explicit operator bool() const noexcept {
            return m_Ptr != nullptr;
        }

        operator UniqueView<const T>() const noexcept {
            return UniqueView<const T>(m_Ptr);
        }
    };

    template<typename T, typename A> requires !std::same_as<A, Allocators::ControlBlockAllocator>
    struct alignas(16) UniquePtr final {
        using allocator_ = A;

    private:
        allocator_* m_Allocator = nullptr;
        T* m_Memory = nullptr;

        template<typename... Args>
        void alloc(Args&&... u_Params) {
            CORIUM_ASSERT(m_Allocator != nullptr);

            T* mem = m_Allocator->template emplace<T>(std::forward<Args>(u_Params)...);

            if (mem) {
                m_Memory = mem;
            } else {
                m_Memory = nullptr;
                CORIUM_ASSERT(false && "Allocation failed in UniquePtr");
            }
        }

        void destroy() {
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
        explicit UniquePtr(allocator_* po_Allocator, Args&&... u_Params) noexcept {
            m_Allocator = po_Allocator;
            alloc(std::forward<Args>(u_Params)...);
        }

        UniqueView<T> view() noexcept {
            return UniqueView<T>(m_Memory);
        }

        UniqueView<const T> view() const noexcept {
            return UniqueView<const T>(m_Memory);
        }

        UniquePtr(const UniquePtr&) = delete;
        UniquePtr& operator=(const UniquePtr&) = delete;

        UniquePtr(UniquePtr&& u_Other) noexcept {
            m_Allocator = u_Other.m_Allocator;
            m_Memory = u_Other.m_Memory;

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


        T* operator->() const noexcept {
            return m_Memory;
        }

        T& operator*() const noexcept {
            return *m_Memory;
        }

        T* get() const noexcept {
            return m_Memory;
        }


        explicit operator bool() const noexcept {
            return m_Memory != nullptr;
        }

        T* release() noexcept {
            T* p = m_Memory;
            m_Memory = nullptr;
            m_Allocator = nullptr;
            return p;
        }

        void reset() noexcept {
            destroy();
            m_Allocator = nullptr;
        }

        template<typename... Args>
        void reset(allocator_* po_Allocator, Args&&... u_Params) {
            destroy();
            m_Allocator = po_Allocator;
            alloc(std::forward<Args>(u_Params)...);
        }
    };


	template<typename T>
	struct CORIUM_RUNTIME_API alignas(16) SharedPtr final {
	};

	template<typename T>
	struct CORIUM_RUNTIME_API alignas(16) WeakPtr final {
	};
}
