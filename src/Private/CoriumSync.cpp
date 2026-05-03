#include "Corium.h"
#include "CoriumSync.h"
#include "CoriumThread.h"
#include "PlatIntrin.h"

namespace Corium::Runtime::Sync {
	CountDownLatch::CountDownLatch(uint32_t v_Count) : m_Counter(v_Count) {}

	void CountDownLatch::countDown() {
		uint32_t next = m_Counter.m_ParkingPermit.decrement(Core::Atomics::MemoryOrder::ACQ_REL);
#if CORIUM_BUILD_DEBUG
		CORIUM_DEBUG_ASSERT(next != UINT32_MAX);
#endif
		if (next == 0) {// Last Thread
			Core::NativeThread::wakeAllOnAddress(m_Counter);
		}
	}

	void CountDownLatch::await() {
		while (true) {
			uint32_t expected = m_Counter.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
			for (uint32_t i = 0; i < CORIUM_SPIN_COUNT; i++) {
				if (expected == 0)
					return;
				Intrinsic::Pause();
				expected = m_Counter.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
			}
			if (expected == 0) return;
			Core::NativeThread::waitOnAddress(m_Counter, expected); // Blocking wait
		}
	}

	void CountDownLatch::await(long duration) {
		// TODO waiting for the waitOnAddressFor function
	}

	uint32_t CountDownLatch::getCount() const {
		return m_Counter.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
	}

	CyclicBarrier::CyclicBarrier(uint32_t v_Parties)
		: m_Generation(0), m_Parties(v_Parties), m_Remaining(v_Parties), m_IsBroken(false) {
	}

	uint32_t CyclicBarrier::await() {
		uint32_t gen = m_Generation.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
		uint32_t rem = m_Remaining.decrement(Core::Atomics::MemoryOrder::ACQ_REL);

		if (rem == 0) {
			m_Remaining.store(m_Parties, Core::Atomics::MemoryOrder::RELEASE);
			m_Generation.m_ParkingPermit.increment(Core::Atomics::MemoryOrder::ACQ_REL);
			Core::NativeThread::wakeAllOnAddress(m_Generation);
			return 0;
		}

		while (m_Generation.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE) == gen && !isBroken())
			Core::NativeThread::waitOnAddress(m_Generation, gen);
		return 0;
	}

	bool CyclicBarrier::await(long v_Duration) {
		return false;
		// TODO
	}

	void CyclicBarrier::reset() {
	}

	bool CyclicBarrier::isBroken() const {
		return m_IsBroken.load(Core::Atomics::MemoryOrder::ACQUIRE);
	}

	uint32_t CyclicBarrier::getParties() const {
		return m_Parties;
	}

	uint32_t CyclicBarrier::getNumberWaiting() const {
		return m_Parties - m_Remaining.load(Core::Atomics::MemoryOrder::ACQUIRE);
	}

	Semaphore::Semaphore(uint32_t v_Permits) : m_Permits(v_Permits) {}

	void Semaphore::acquire() {
		while (true) {
			uint32_t v = m_Permits.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (v > 0) {
				uint32_t old = m_Permits.m_ParkingPermit.compareExchange(&v, v - 1, Core::Atomics::MemoryOrder::ACQ_REL, Core::Atomics::MemoryOrder::RELAXED);
				if (old == v) return;
			}
			for (auto i = 0; i < CORIUM_SPIN_COUNT; i++) {
				v = m_Permits.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
				if (v > 0 && m_Permits.m_ParkingPermit.compareExchange(&v, v - 1, Core::Atomics::MemoryOrder::ACQ_REL, Core::Atomics::MemoryOrder::RELAXED) == v)
					return;

				Intrinsic::Pause();
			}
			v = m_Permits.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (v == 0) Core::NativeThread::waitOnAddress(m_Permits, 0);
		}
	}

	void Semaphore::acquire(uint32_t v_Permits) {
		while (true) {
			uint32_t v = m_Permits.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (v >= v_Permits) {
				uint32_t old = m_Permits.m_ParkingPermit.compareExchange(&v, v - v_Permits, Core::Atomics::MemoryOrder::ACQ_REL, Core::Atomics::MemoryOrder::RELAXED);
				if (old == v) return;
			}
			for (auto i = 0; i < CORIUM_SPIN_COUNT; i++) {
				v = m_Permits.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
				if (v >= v_Permits && m_Permits.m_ParkingPermit.compareExchange(&v, v - v_Permits, Core::Atomics::MemoryOrder::ACQ_REL, Core::Atomics::MemoryOrder::RELAXED) == v)
					return;

				Intrinsic::Pause();
			}
			v = m_Permits.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (v < v_Permits) Core::NativeThread::waitOnAddress(m_Permits, v);
		}
	}

	bool Semaphore::tryAcquire() {
		uint32_t v = m_Permits.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
		if (v > 0) {
			uint32_t old = m_Permits.m_ParkingPermit.compareExchange(&v, v - 1, Core::Atomics::MemoryOrder::ACQ_REL, Core::Atomics::MemoryOrder::RELAXED);
			if (old == v) return true;
		}
		return false;
	}

	bool Semaphore::tryAcquire(long v_Timeout) {
		return true; // TODO
	}

	void Semaphore::release() {
		uint32_t prev = m_Permits.m_ParkingPermit.fetchAdd(1, Core::Atomics::MemoryOrder::ACQ_REL);
		if (prev == 0) Core::NativeThread::wakeOnAddress(m_Permits);
	}

	void Semaphore::release(uint32_t v_Permits) {
		uint32_t prev = m_Permits.m_ParkingPermit.fetchAdd(v_Permits, Core::Atomics::MemoryOrder::ACQ_REL);
		Core::NativeThread::wakeAllOnAddress(m_Permits);
	}

	uint32_t Semaphore::availablePermits() const {
		return m_Permits.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
	}
}