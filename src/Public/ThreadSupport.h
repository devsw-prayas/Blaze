#pragma once
#include <cstddef>
#include <cstring>
#include <cstdint>

#include "CoriumDiagnostics.h"
#include "ThreadUtils.h"
#include "CoriumChrono.h"

namespace Corium::Core {

	constexpr size_t CORIUM_SUPPORT_STORAGE_SIZE = 64;

	// HandleSupport
	//
	// Opaque, fixed 64B (one cache line) storage for any thread-handle-shaped type H
	// (alignof(H) == 64, sizeof(H) <= 64). ThreadHandle is fully opaque outside NativeThread,
	// so the only thing a generic caller can meaningfully ask of a stored handle is its
	// claimed state — captured once, at construction, as a single type-erased accessor.
	class HandleSupport final {
	private:
		alignas(CORIUM_SUPPORT_STORAGE_SIZE) std::byte m_Storage[CORIUM_SUPPORT_STORAGE_SIZE];

		using StateFn = uint32_t(*)(std::byte*);
		StateFn m_GetState = nullptr;

	public:
		HandleSupport() {
			ThreadHandle v_Default = ThreadHandle::getInvalidThread();
			memcpy(m_Storage, &v_Default, sizeof(ThreadHandle));
			m_GetState = [](std::byte* p) {
				return static_cast<uint32_t>(reinterpret_cast<ThreadHandle*>(p)->expectedState());
			};
		}

		template<typename H>
		explicit HandleSupport(H* p_Handle) {
			CORIUM_STATIC_ASSERT(alignof(H) <= CORIUM_SUPPORT_STORAGE_SIZE, "Invalid alignment of incoming handle");
			CORIUM_STATIC_ASSERT(sizeof(H) <= CORIUM_SUPPORT_STORAGE_SIZE, "Incoming handle does not fit in storage");
			memcpy(m_Storage, p_Handle, sizeof(H));
			m_GetState = [](std::byte* p) {
				return static_cast<uint32_t>(reinterpret_cast<H*>(p)->expectedState());
			};
		}

		template<typename H>
		H* get() {
			CORIUM_STATIC_ASSERT(alignof(H) <= CORIUM_SUPPORT_STORAGE_SIZE, "Invalid alignment of requested handle");
			CORIUM_STATIC_ASSERT(sizeof(H) <= CORIUM_SUPPORT_STORAGE_SIZE, "Requested handle does not fit in storage");
			return reinterpret_cast<H*>(m_Storage);
		}

		template<typename H>
		const H* get() const {
			CORIUM_STATIC_ASSERT(alignof(H) <= CORIUM_SUPPORT_STORAGE_SIZE, "Invalid alignment of requested handle");
			CORIUM_STATIC_ASSERT(sizeof(H) <= CORIUM_SUPPORT_STORAGE_SIZE, "Requested handle does not fit in storage");
			return reinterpret_cast<const H*>(m_Storage);
		}

		uint32_t getState() const noexcept {
			return m_GetState(const_cast<std::byte*>(m_Storage));
		}
	};

	// ParkingSupport
	//
	// Opaque, fixed 64B (one cache line) storage for any park-handle-shaped type H exposing
	// an m_ParkingPermit atomic (AtomicValue32<uint32_t>-shaped). The atomic primitives
	// (data/load/store/decrement/increment/compareExchange/fetchAdd) are captured once,
	// per-type, at construction — that's the only part that varies across handle types.
	// The actual wait/wake syscalls live on NativeThread, which takes a ParkingSupport&
	// directly and reaches in via data()/store() — they don't depend on the handle type
	// backing this storage, so they don't belong here.
	class ParkingSupport final {
	private:
		alignas(CORIUM_SUPPORT_STORAGE_SIZE) std::byte m_Storage[CORIUM_SUPPORT_STORAGE_SIZE];

		using DataFn     = uint32_t*(*)(std::byte*);
		using LoadFn     = uint32_t(*)(std::byte*, Atomics::MemoryOrder);
		using StoreFn    = void(*)(std::byte*, uint32_t, Atomics::MemoryOrder);
		using IncrFn     = uint32_t(*)(std::byte*, Atomics::MemoryOrder);
		using CasFn      = uint32_t(*)(std::byte*, uint32_t*, uint32_t, Atomics::MemoryOrder, Atomics::MemoryOrder);
		using FetchAddFn = uint32_t(*)(std::byte*, uint32_t, Atomics::MemoryOrder);

		DataFn     m_Data            = nullptr;
		LoadFn     m_Load            = nullptr;
		StoreFn    m_Store           = nullptr;
		IncrFn     m_Decrement       = nullptr;
		IncrFn     m_Increment       = nullptr;
		CasFn      m_CompareExchange = nullptr;
		FetchAddFn m_FetchAdd        = nullptr;

	public:
		ParkingSupport() {
			ParkHandle v_Default{ 0u };
			memcpy(m_Storage, &v_Default, sizeof(ParkHandle));

			m_Data = [](std::byte* p) {
				return reinterpret_cast<ParkHandle*>(p)->m_ParkingPermit.data();
			};
			m_Load = [](std::byte* p, Atomics::MemoryOrder v_Ordering) {
				return reinterpret_cast<ParkHandle*>(p)->m_ParkingPermit.load(v_Ordering);
			};
			m_Store = [](std::byte* p, uint32_t v_Value, Atomics::MemoryOrder v_Ordering) {
				reinterpret_cast<ParkHandle*>(p)->m_ParkingPermit.store(v_Value, v_Ordering);
			};
			m_Decrement = [](std::byte* p, Atomics::MemoryOrder v_Ordering) {
				return reinterpret_cast<ParkHandle*>(p)->m_ParkingPermit.decrement(v_Ordering);
			};
			m_Increment = [](std::byte* p, Atomics::MemoryOrder v_Ordering) {
				return reinterpret_cast<ParkHandle*>(p)->m_ParkingPermit.increment(v_Ordering);
			};
			m_CompareExchange = [](std::byte* p, uint32_t* p_Expected, uint32_t v_Desired,
									Atomics::MemoryOrder v_Success, Atomics::MemoryOrder v_Failure) {
				return reinterpret_cast<ParkHandle*>(p)->m_ParkingPermit.compareExchange(
					p_Expected, v_Desired, v_Success, v_Failure);
			};
			m_FetchAdd = [](std::byte* p, uint32_t v_Value, Atomics::MemoryOrder v_Ordering) {
				return reinterpret_cast<ParkHandle*>(p)->m_ParkingPermit.fetchAdd(v_Value, v_Ordering);
			};
		}

		template<typename H>
		explicit ParkingSupport(H* p_Handle) {
			CORIUM_STATIC_ASSERT(alignof(H) <= CORIUM_SUPPORT_STORAGE_SIZE, "Invalid alignment of incoming handle");
			CORIUM_STATIC_ASSERT(sizeof(H) <= CORIUM_SUPPORT_STORAGE_SIZE, "Incoming handle does not fit in storage");
			memcpy(m_Storage, p_Handle, sizeof(H));

			m_Data = [](std::byte* p) {
				return reinterpret_cast<H*>(p)->m_ParkingPermit.data();
			};
			m_Load = [](std::byte* p, Atomics::MemoryOrder v_Ordering) {
				return reinterpret_cast<H*>(p)->m_ParkingPermit.load(v_Ordering);
			};
			m_Store = [](std::byte* p, uint32_t v_Value, Atomics::MemoryOrder v_Ordering) {
				reinterpret_cast<H*>(p)->m_ParkingPermit.store(v_Value, v_Ordering);
			};
			m_Decrement = [](std::byte* p, Atomics::MemoryOrder v_Ordering) {
				return reinterpret_cast<H*>(p)->m_ParkingPermit.decrement(v_Ordering);
			};
			m_Increment = [](std::byte* p, Atomics::MemoryOrder v_Ordering) {
				return reinterpret_cast<H*>(p)->m_ParkingPermit.increment(v_Ordering);
			};
			m_CompareExchange = [](std::byte* p, uint32_t* p_Expected, uint32_t v_Desired,
									Atomics::MemoryOrder v_Success, Atomics::MemoryOrder v_Failure) {
				return reinterpret_cast<H*>(p)->m_ParkingPermit.compareExchange(
					p_Expected, v_Desired, v_Success, v_Failure);
			};
			m_FetchAdd = [](std::byte* p, uint32_t v_Value, Atomics::MemoryOrder v_Ordering) {
				return reinterpret_cast<H*>(p)->m_ParkingPermit.fetchAdd(v_Value, v_Ordering);
			};
		}

		template<typename H>
		H* get() {
			CORIUM_STATIC_ASSERT(alignof(H) <= CORIUM_SUPPORT_STORAGE_SIZE, "Invalid alignment of requested handle");
			CORIUM_STATIC_ASSERT(sizeof(H) <= CORIUM_SUPPORT_STORAGE_SIZE, "Requested handle does not fit in storage");
			return reinterpret_cast<H*>(m_Storage);
		}

		template<typename H>
		const H* get() const {
			CORIUM_STATIC_ASSERT(alignof(H) <= CORIUM_SUPPORT_STORAGE_SIZE, "Invalid alignment of requested handle");
			CORIUM_STATIC_ASSERT(sizeof(H) <= CORIUM_SUPPORT_STORAGE_SIZE, "Requested handle does not fit in storage");
			return reinterpret_cast<const H*>(m_Storage);
		}

		uint32_t* data() noexcept { return m_Data(m_Storage); }

		uint32_t load(Atomics::MemoryOrder v_Ordering = Atomics::MemoryOrder::SEQ_CST) const noexcept {
			return m_Load(const_cast<std::byte*>(m_Storage), v_Ordering);
		}

		void store(uint32_t v_Value, Atomics::MemoryOrder v_Ordering = Atomics::MemoryOrder::SEQ_CST) noexcept {
			m_Store(m_Storage, v_Value, v_Ordering);
		}

		uint32_t decrement(Atomics::MemoryOrder v_Ordering = Atomics::MemoryOrder::SEQ_CST) noexcept {
			return m_Decrement(m_Storage, v_Ordering);
		}

		uint32_t increment(Atomics::MemoryOrder v_Ordering = Atomics::MemoryOrder::SEQ_CST) noexcept {
			return m_Increment(m_Storage, v_Ordering);
		}

		uint32_t compareExchange(uint32_t* p_Expected, uint32_t v_Desired,
								  Atomics::MemoryOrder v_Success, Atomics::MemoryOrder v_Failure) noexcept {
			return m_CompareExchange(m_Storage, p_Expected, v_Desired, v_Success, v_Failure);
		}

		uint32_t fetchAdd(uint32_t v_Value, Atomics::MemoryOrder v_Ordering = Atomics::MemoryOrder::SEQ_CST) noexcept {
			return m_FetchAdd(m_Storage, v_Value, v_Ordering);
		}
	};
}
