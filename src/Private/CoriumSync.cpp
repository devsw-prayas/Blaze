#include "Corium.h"

#define ALLOW_SYSCALL
#include "CoriumSyscalls.h"
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

	bool CountDownLatch::await(Chrono::Instant v_Deadline) {
		while (true) {
			uint32_t expected = m_Counter.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
			for (uint32_t i = 0; i < CORIUM_SPIN_COUNT; ++i) {
				if (expected == 0) return true;
				Intrinsic::Pause();
				expected = m_Counter.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
			}
			if (expected == 0) return true;
			if (v_Deadline.isExpired()) return false;
			Core::NativeThread::waitOnAddressFor(m_Counter, v_Deadline);
		}
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

	bool CyclicBarrier::await(Chrono::Instant v_Deadline) {
		uint32_t gen = m_Generation.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
		uint32_t rem = m_Remaining.decrement(Core::Atomics::MemoryOrder::ACQ_REL);

		if (rem == 0) {
			m_Remaining.store(m_Parties, Core::Atomics::MemoryOrder::RELEASE);
			m_Generation.m_ParkingPermit.increment(Core::Atomics::MemoryOrder::ACQ_REL);
			Core::NativeThread::wakeAllOnAddress(m_Generation);
			return true;
		}

		while (m_Generation.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE) == gen && !isBroken()) {
			if (v_Deadline.isExpired()) {
				m_IsBroken.store(true, Core::Atomics::MemoryOrder::RELEASE);
				Core::NativeThread::wakeAllOnAddress(m_Generation);
				return false;
			}
			Core::NativeThread::waitOnAddressFor(m_Generation, v_Deadline);
		}
		return !isBroken();
	}

	void CyclicBarrier::reset() {
		m_Remaining.store(m_Parties, Core::Atomics::MemoryOrder::RELEASE);
		m_IsBroken.store(false, Core::Atomics::MemoryOrder::RELEASE);
		m_Generation.m_ParkingPermit.increment(Core::Atomics::MemoryOrder::ACQ_REL);
		Core::NativeThread::wakeAllOnAddress(m_Generation);
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

	bool Semaphore::tryAcquire(Chrono::Instant v_Deadline) {
		while (true) {
			uint32_t v = m_Permits.m_ParkingPermit.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (v > 0) {
				uint32_t old = m_Permits.m_ParkingPermit.compareExchange(&v, v - 1, Core::Atomics::MemoryOrder::ACQ_REL, Core::Atomics::MemoryOrder::RELAXED);
				if (old == v) return true;
			}
			if (v_Deadline.isExpired()) return false;
			Core::NativeThread::waitOnAddressFor(m_Permits, v_Deadline);
		}
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

	// ─── CriticalSection ──────────────────────────────────────────────────────

#ifdef _WIN32
	static_assert(sizeof(CRITICAL_SECTION) <= CriticalSection::STORAGE_SIZE,
				  "CRITICAL_SECTION exceeds CriticalSection storage buffer");
#elif defined(__linux__)
	static_assert(sizeof(pthread_mutex_t) <= CriticalSection::STORAGE_SIZE,
				  "pthread_mutex_t exceeds CriticalSection storage buffer");
#endif

	CriticalSection::CriticalSection() noexcept {
#ifdef _WIN32
		InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_Storage));
#elif defined(__linux__)
		pthread_mutexattr_t v_Attr;
		pthread_mutexattr_init(&v_Attr);
		pthread_mutexattr_settype(&v_Attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(reinterpret_cast<pthread_mutex_t*>(m_Storage), &v_Attr);
		pthread_mutexattr_destroy(&v_Attr);
#endif
	}

	CriticalSection::~CriticalSection() {
#ifdef _WIN32
		DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_Storage));
#elif defined(__linux__)
		pthread_mutex_destroy(reinterpret_cast<pthread_mutex_t*>(m_Storage));
#endif
	}

	void CriticalSection::lock() noexcept {
#ifdef _WIN32
		EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_Storage));
#elif defined(__linux__)
		pthread_mutex_lock(reinterpret_cast<pthread_mutex_t*>(m_Storage));
#endif
	}

	void CriticalSection::unlock() noexcept {
#ifdef _WIN32
		LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_Storage));
#elif defined(__linux__)
		pthread_mutex_unlock(reinterpret_cast<pthread_mutex_t*>(m_Storage));
#endif
	}

	bool CriticalSection::tryLock() noexcept {
#ifdef _WIN32
		return TryEnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(m_Storage)) != FALSE;
#elif defined(__linux__)
		return pthread_mutex_trylock(reinterpret_cast<pthread_mutex_t*>(m_Storage)) == 0;
#else
		return false;
#endif
	}

	// ─── ReentrantLock ────────────────────────────────────────────────────────

	namespace {
		CORIUM_FORCEINLINE uint32_t currentTid() noexcept {
#ifdef _WIN32
			return static_cast<uint32_t>(GetCurrentThreadId());
#elif defined(__linux__)
			return static_cast<uint32_t>(pthread_self());
#else
			return 0u;
#endif
		}
	}

	void ReentrantLock::lock() noexcept {
		const uint32_t v_MyTid = currentTid();

		if (m_OwnerTid.load(Core::Atomics::MemoryOrder::ACQUIRE) == v_MyTid) {
			m_HoldCount.increment(Core::Atomics::MemoryOrder::RELAXED);
			return;
		}

		for (int i = 0; i < CORIUM_SPIN_COUNT; ++i) {
			uint32_t v_Expected = 0u;
			if (m_OwnerTid.compareExchange(&v_Expected, v_MyTid,
					Core::Atomics::MemoryOrder::ACQ_REL,
					Core::Atomics::MemoryOrder::RELAXED) == 0u) {
				m_HoldCount.store(1u, Core::Atomics::MemoryOrder::RELAXED);
				return;
			}
			Intrinsic::Pause();
		}

		while (true) {
			uint32_t v_Expected = 0u;
			if (m_OwnerTid.compareExchange(&v_Expected, v_MyTid,
					Core::Atomics::MemoryOrder::ACQ_REL,
					Core::Atomics::MemoryOrder::RELAXED) == 0u) {
				m_HoldCount.store(1u, Core::Atomics::MemoryOrder::RELAXED);
				return;
			}
			uint32_t v_Gate = m_Gate.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (m_OwnerTid.load(Core::Atomics::MemoryOrder::ACQUIRE) == 0u) continue;
#ifdef _WIN32
			WaitOnAddress(m_Gate.data(), &v_Gate, sizeof(uint32_t), INFINITE);
#else
			// TODO: Linux futex
#endif
		}
	}

	void ReentrantLock::unlock() noexcept {
		const uint32_t v_Remaining = m_HoldCount.decrement(Core::Atomics::MemoryOrder::ACQ_REL);
		if (v_Remaining > 0u) return;
		m_OwnerTid.store(0u, Core::Atomics::MemoryOrder::RELEASE);
		m_Gate.fetchAdd(1u, Core::Atomics::MemoryOrder::ACQ_REL);
#ifdef _WIN32
		WakeByAddressSingle(m_Gate.data());
#else
		// TODO: Linux futex_wake
#endif
	}

	bool ReentrantLock::tryLock() noexcept {
		const uint32_t v_MyTid = currentTid();
		if (m_OwnerTid.load(Core::Atomics::MemoryOrder::ACQUIRE) == v_MyTid) {
			m_HoldCount.increment(Core::Atomics::MemoryOrder::RELAXED);
			return true;
		}
		uint32_t v_Expected = 0u;
		if (m_OwnerTid.compareExchange(&v_Expected, v_MyTid,
				Core::Atomics::MemoryOrder::ACQ_REL,
				Core::Atomics::MemoryOrder::RELAXED) == 0u) {
			m_HoldCount.store(1u, Core::Atomics::MemoryOrder::RELAXED);
			return true;
		}
		return false;
	}

	bool ReentrantLock::tryLock(Chrono::Instant v_Deadline) noexcept {
		const uint32_t v_MyTid = currentTid();
		if (m_OwnerTid.load(Core::Atomics::MemoryOrder::ACQUIRE) == v_MyTid) {
			m_HoldCount.increment(Core::Atomics::MemoryOrder::RELAXED);
			return true;
		}
		while (true) {
			uint32_t v_Expected = 0u;
			if (m_OwnerTid.compareExchange(&v_Expected, v_MyTid,
					Core::Atomics::MemoryOrder::ACQ_REL,
					Core::Atomics::MemoryOrder::RELAXED) == 0u) {
				m_HoldCount.store(1u, Core::Atomics::MemoryOrder::RELAXED);
				return true;
			}
			if (v_Deadline.isExpired()) return false;
			uint32_t v_Gate = m_Gate.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (m_OwnerTid.load(Core::Atomics::MemoryOrder::ACQUIRE) == 0u) continue;
#ifdef _WIN32
			const int64_t v_Ms = v_Deadline.remainingMilliseconds();
			const DWORD v_Timeout = (v_Ms <= 0) ? 0u
				: static_cast<DWORD>(v_Ms < 0xFFFFFFFELL ? v_Ms : 0xFFFFFFFEu);
			WaitOnAddress(m_Gate.data(), &v_Gate, sizeof(uint32_t), v_Timeout);
#else
			// TODO: Linux futex with timeout
			return false;
#endif
		}
	}

	bool ReentrantLock::isHeldByCurrentThread() const noexcept {
		return m_OwnerTid.load(Core::Atomics::MemoryOrder::ACQUIRE) == currentTid();
	}

	uint32_t ReentrantLock::getHoldCount() const noexcept {
		if (!isHeldByCurrentThread()) return 0u;
		return m_HoldCount.load(Core::Atomics::MemoryOrder::RELAXED);
	}

	// ─── ReadWriteLock ───────────────────────────────────────────────────────

	static constexpr uint32_t RWL_WRITE_LOCKED = ~0u;

	void ReadWriteLock::lockRead() noexcept {
		for (;;) {
			for (int i = 0; i < CORIUM_SPIN_COUNT; ++i) {
				uint32_t v_State = m_State.load(Core::Atomics::MemoryOrder::ACQUIRE);
				if (v_State != RWL_WRITE_LOCKED) {
					if (m_State.compareExchange(&v_State, v_State + 1,
							Core::Atomics::MemoryOrder::ACQ_REL,
							Core::Atomics::MemoryOrder::RELAXED) == v_State) return;
				}
				Intrinsic::Pause();
			}
			uint32_t v_Gate = m_Gate.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (m_State.load(Core::Atomics::MemoryOrder::ACQUIRE) != RWL_WRITE_LOCKED) continue;
#ifdef _WIN32
			WaitOnAddress(m_Gate.data(), &v_Gate, sizeof(uint32_t), INFINITE);
#else
			// TODO: Linux futex
#endif
		}
	}

	bool ReadWriteLock::tryLockRead() noexcept {
		uint32_t v_State = m_State.load(Core::Atomics::MemoryOrder::ACQUIRE);
		if (v_State == RWL_WRITE_LOCKED) return false;
		return m_State.compareExchange(&v_State, v_State + 1,
			Core::Atomics::MemoryOrder::ACQ_REL,
			Core::Atomics::MemoryOrder::RELAXED) == v_State;
	}

	bool ReadWriteLock::tryLockRead(Chrono::Instant v_Deadline) noexcept {
		for (;;) {
			uint32_t v_State = m_State.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (v_State != RWL_WRITE_LOCKED) {
				if (m_State.compareExchange(&v_State, v_State + 1,
						Core::Atomics::MemoryOrder::ACQ_REL,
						Core::Atomics::MemoryOrder::RELAXED) == v_State) return true;
				continue;
			}
			if (v_Deadline.isExpired()) return false;
			uint32_t v_Gate = m_Gate.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (m_State.load(Core::Atomics::MemoryOrder::ACQUIRE) != RWL_WRITE_LOCKED) continue;
#ifdef _WIN32
			const int64_t v_Ms = v_Deadline.remainingMilliseconds();
			const DWORD v_Timeout = (v_Ms <= 0) ? 0u
				: static_cast<DWORD>(v_Ms < 0xFFFFFFFELL ? v_Ms : 0xFFFFFFFEu);
			WaitOnAddress(m_Gate.data(), &v_Gate, sizeof(uint32_t), v_Timeout);
#else
			// TODO: Linux futex with timeout
			return false;
#endif
		}
	}

	void ReadWriteLock::unlockRead() noexcept {
		const uint32_t v_New = m_State.decrement(Core::Atomics::MemoryOrder::ACQ_REL);
		if (v_New == 0) {
			m_Gate.fetchAdd(1u, Core::Atomics::MemoryOrder::ACQ_REL);
#ifdef _WIN32
			WakeByAddressSingle(m_Gate.data()); // only one writer can win
#else
			// TODO: Linux futex_wake
#endif
		}
	}

	void ReadWriteLock::lockWrite() noexcept {
		for (;;) {
			for (int i = 0; i < CORIUM_SPIN_COUNT; ++i) {
				uint32_t v_Expected = 0u;
				if (m_State.compareExchange(&v_Expected, RWL_WRITE_LOCKED,
						Core::Atomics::MemoryOrder::ACQ_REL,
						Core::Atomics::MemoryOrder::RELAXED) == 0u) return;
				Intrinsic::Pause();
			}
			uint32_t v_Gate = m_Gate.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (m_State.load(Core::Atomics::MemoryOrder::ACQUIRE) == 0u) continue;
#ifdef _WIN32
			WaitOnAddress(m_Gate.data(), &v_Gate, sizeof(uint32_t), INFINITE);
#else
			// TODO: Linux futex
#endif
		}
	}

	bool ReadWriteLock::tryLockWrite() noexcept {
		uint32_t v_Expected = 0u;
		return m_State.compareExchange(&v_Expected, RWL_WRITE_LOCKED,
			Core::Atomics::MemoryOrder::ACQ_REL,
			Core::Atomics::MemoryOrder::RELAXED) == 0u;
	}

	bool ReadWriteLock::tryLockWrite(Chrono::Instant v_Deadline) noexcept {
		for (;;) {
			uint32_t v_Expected = 0u;
			if (m_State.compareExchange(&v_Expected, RWL_WRITE_LOCKED,
					Core::Atomics::MemoryOrder::ACQ_REL,
					Core::Atomics::MemoryOrder::RELAXED) == 0u) return true;
			if (v_Deadline.isExpired()) return false;
			uint32_t v_Gate = m_Gate.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (m_State.load(Core::Atomics::MemoryOrder::ACQUIRE) == 0u) continue;
#ifdef _WIN32
			const int64_t v_Ms = v_Deadline.remainingMilliseconds();
			const DWORD v_Timeout = (v_Ms <= 0) ? 0u
				: static_cast<DWORD>(v_Ms < 0xFFFFFFFELL ? v_Ms : 0xFFFFFFFEu);
			WaitOnAddress(m_Gate.data(), &v_Gate, sizeof(uint32_t), v_Timeout);
#else
			// TODO: Linux futex with timeout
			return false;
#endif
		}
	}

	void ReadWriteLock::unlockWrite() noexcept {
		m_State.store(0u, Core::Atomics::MemoryOrder::RELEASE);
		m_Gate.fetchAdd(1u, Core::Atomics::MemoryOrder::ACQ_REL);
#ifdef _WIN32
		WakeByAddressAll(m_Gate.data()); // readers and writers can all compete
#else
		// TODO: Linux futex_wake broadcast
#endif
	}

	bool ReadWriteLock::isWriteLocked() const noexcept {
		return m_State.load(Core::Atomics::MemoryOrder::ACQUIRE) == RWL_WRITE_LOCKED;
	}

	uint32_t ReadWriteLock::getReadLockCount() const noexcept {
		const uint32_t v_State = m_State.load(Core::Atomics::MemoryOrder::ACQUIRE);
		return (v_State == RWL_WRITE_LOCKED) ? 0u : v_State;
	}

	// ─── StampedLock ─────────────────────────────────────────────────────────

	static constexpr uint64_t SL_WBIT    = 0x80ULL;   // write lock bit
	static constexpr uint64_t SL_RBITS   = 0x7FULL;   // reader count mask (max 127)
	static constexpr uint64_t SL_ABITS   = 0xFFULL;   // all lock bits
	static constexpr uint64_t SL_VER_INC = 0x100ULL;  // version step per write cycle
	static constexpr uint64_t SL_SBITS   = ~SL_ABITS; // version/stamp bits

	uint64_t StampedLock::writeLock() noexcept {
		for (;;) {
			for (int i = 0; i < CORIUM_SPIN_COUNT; ++i) {
				uint64_t v_State = m_State.load(Core::Atomics::MemoryOrder::ACQUIRE);
				if ((v_State & SL_ABITS) == 0) {
					uint64_t v_Next = v_State | SL_WBIT;
					if (m_State.compareExchange(&v_State, v_Next,
							Core::Atomics::MemoryOrder::ACQ_REL,
							Core::Atomics::MemoryOrder::RELAXED) == v_State) return v_Next;
				}
				Intrinsic::Pause();
			}
			uint32_t v_Gate = m_Gate.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if ((m_State.load(Core::Atomics::MemoryOrder::ACQUIRE) & SL_ABITS) == 0) continue;
#ifdef _WIN32
			WaitOnAddress(m_Gate.data(), &v_Gate, sizeof(uint32_t), INFINITE);
#else
			// TODO: Linux futex
#endif
		}
	}

	bool StampedLock::tryWriteLock(uint64_t& ro_Stamp) noexcept {
		uint64_t v_State = m_State.load(Core::Atomics::MemoryOrder::ACQUIRE);
		if ((v_State & SL_ABITS) == 0) {
			uint64_t v_Next = v_State | SL_WBIT;
			if (m_State.compareExchange(&v_State, v_Next,
					Core::Atomics::MemoryOrder::ACQ_REL,
					Core::Atomics::MemoryOrder::RELAXED) == v_State) {
				ro_Stamp = v_Next;
				return true;
			}
		}
		return false;
	}

	void StampedLock::unlockWrite(uint64_t v_Stamp) noexcept {
		// Bump version, clear all lock bits
		m_State.store((v_Stamp & SL_SBITS) + SL_VER_INC, Core::Atomics::MemoryOrder::RELEASE);
		m_Gate.fetchAdd(1u, Core::Atomics::MemoryOrder::ACQ_REL);
#ifdef _WIN32
		WakeByAddressAll(m_Gate.data());
#else
		// TODO: Linux futex_wake broadcast
#endif
	}

	uint64_t StampedLock::readLock() noexcept {
		for (;;) {
			for (int i = 0; i < CORIUM_SPIN_COUNT; ++i) {
				uint64_t v_State = m_State.load(Core::Atomics::MemoryOrder::ACQUIRE);
				if (!(v_State & SL_WBIT) && (v_State & SL_RBITS) < SL_RBITS) {
					uint64_t v_Next = v_State + 1;
					if (m_State.compareExchange(&v_State, v_Next,
							Core::Atomics::MemoryOrder::ACQ_REL,
							Core::Atomics::MemoryOrder::RELAXED) == v_State) return v_Next;
				}
				Intrinsic::Pause();
			}
			uint32_t v_Gate = m_Gate.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (!(m_State.load(Core::Atomics::MemoryOrder::ACQUIRE) & SL_WBIT)) continue;
#ifdef _WIN32
			WaitOnAddress(m_Gate.data(), &v_Gate, sizeof(uint32_t), INFINITE);
#else
			// TODO: Linux futex
#endif
		}
	}

	bool StampedLock::tryReadLock(uint64_t& ro_Stamp) noexcept {
		uint64_t v_State = m_State.load(Core::Atomics::MemoryOrder::ACQUIRE);
		if (!(v_State & SL_WBIT) && (v_State & SL_RBITS) < SL_RBITS) {
			uint64_t v_Next = v_State + 1;
			if (m_State.compareExchange(&v_State, v_Next,
					Core::Atomics::MemoryOrder::ACQ_REL,
					Core::Atomics::MemoryOrder::RELAXED) == v_State) {
				ro_Stamp = v_Next;
				return true;
			}
		}
		return false;
	}

	void StampedLock::unlockRead(uint64_t v_Stamp) noexcept {
		CORIUM_MAYBE_UNUSED uint64_t v_New = m_State.decrement(Core::Atomics::MemoryOrder::ACQ_REL);
		if ((v_New & SL_ABITS) == 0) {
			m_Gate.fetchAdd(1u, Core::Atomics::MemoryOrder::ACQ_REL);
#ifdef _WIN32
			WakeByAddressSingle(m_Gate.data()); // last reader: wake one waiting writer
#else
			// TODO: Linux futex_wake
#endif
		}
		CORIUM_MAYBE_UNUSED uint64_t v_Unused = v_Stamp; // stamp reserved for future validation
	}

	uint64_t StampedLock::tryOptimisticRead() noexcept {
		uint64_t v_State = m_State.load(Core::Atomics::MemoryOrder::ACQUIRE);
		return (v_State & SL_WBIT) ? 0ULL : (v_State & SL_SBITS);
	}

	bool StampedLock::validate(uint64_t v_Stamp) const noexcept {
		uint64_t v_State = m_State.load(Core::Atomics::MemoryOrder::ACQUIRE);
		return !(v_State & SL_WBIT) && (v_State & SL_SBITS) == v_Stamp;
	}

	uint64_t StampedLock::tryConvertToWriteLock(uint64_t v_Stamp) noexcept {
		// Write stamp: already own write lock, validate and return
		if (v_Stamp & SL_WBIT) {
			return (m_State.load(Core::Atomics::MemoryOrder::ACQUIRE) == v_Stamp) ? v_Stamp : 0ULL;
		}
		// Read stamp: upgrade only if we are the sole reader
		if (v_Stamp & SL_RBITS) {
			uint64_t v_Expected = v_Stamp;
			uint64_t v_Next = (v_Stamp & SL_SBITS) | SL_WBIT; // drop reader count, set write bit
			return (m_State.compareExchange(&v_Expected, v_Next,
				Core::Atomics::MemoryOrder::ACQ_REL,
				Core::Atomics::MemoryOrder::RELAXED) == v_Stamp) ? v_Next : 0ULL;
		}
		// Optimistic stamp: acquire write lock if version matches and state is unlocked
		for (;;) {
			uint64_t v_State = m_State.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if ((v_State & SL_SBITS) != v_Stamp) return 0ULL; // writer intervened
			if ((v_State & SL_ABITS) != 0) return 0ULL;       // locked by someone
			uint64_t v_Next = v_State | SL_WBIT;
			if (m_State.compareExchange(&v_State, v_Next,
					Core::Atomics::MemoryOrder::ACQ_REL,
					Core::Atomics::MemoryOrder::RELAXED) == v_State) return v_Next;
		}
	}

	uint64_t StampedLock::tryConvertToReadLock(uint64_t v_Stamp) noexcept {
		// Write stamp: downgrade to read
		if (v_Stamp & SL_WBIT) {
			uint64_t v_Next = (v_Stamp & SL_SBITS) | 1ULL; // clear write bit, add 1 reader
			if (m_State.compareExchange(&v_Stamp, v_Next,
					Core::Atomics::MemoryOrder::ACQ_REL,
					Core::Atomics::MemoryOrder::RELAXED) == v_Stamp) {
				// Wake any waiting readers — we released the write lock
				m_Gate.fetchAdd(1u, Core::Atomics::MemoryOrder::ACQ_REL);
#ifdef _WIN32
				WakeByAddressAll(m_Gate.data());
#else
				// TODO: Linux futex_wake broadcast
#endif
				return v_Next;
			}
			return 0ULL;
		}
		// Read stamp: already a reader, validate version is still current
		if (v_Stamp & SL_RBITS) {
			uint64_t v_State = m_State.load(Core::Atomics::MemoryOrder::ACQUIRE);
			return ((v_State & SL_SBITS) == (v_Stamp & SL_SBITS) && !(v_State & SL_WBIT))
				? v_Stamp : 0ULL;
		}
		// Optimistic stamp: acquire read lock if version still matches and no writer
		uint64_t v_State = m_State.load(Core::Atomics::MemoryOrder::ACQUIRE);
		if ((v_State & SL_SBITS) != v_Stamp || (v_State & SL_WBIT)) return 0ULL;
		if ((v_State & SL_RBITS) >= SL_RBITS) return 0ULL;
		uint64_t v_Next = v_State + 1;
		return (m_State.compareExchange(&v_State, v_Next,
			Core::Atomics::MemoryOrder::ACQ_REL,
			Core::Atomics::MemoryOrder::RELAXED) == v_State) ? v_Next : 0ULL;
	}

	// ─── Condition ────────────────────────────────────────────────────────────

	void Condition::await() noexcept {
		const uint32_t v_HoldCount = m_Lock.getHoldCount();
		// Read seq while still holding the lock so we can't miss a signal.
		const uint32_t v_Seq = m_Seq.load(Core::Atomics::MemoryOrder::ACQUIRE);

		for (uint32_t i = 0; i < v_HoldCount; ++i)
			m_Lock.unlock();

#ifdef _WIN32
		WaitOnAddress(m_Seq.data(), const_cast<uint32_t*>(&v_Seq), sizeof(uint32_t), INFINITE);
#else
		// TODO: Linux futex
#endif

		m_Lock.lock();
		for (uint32_t i = 1; i < v_HoldCount; ++i)
			m_Lock.lock();
	}

	bool Condition::await(Chrono::Instant v_Deadline) noexcept {
		const uint32_t v_HoldCount = m_Lock.getHoldCount();
		const uint32_t v_Seq       = m_Seq.load(Core::Atomics::MemoryOrder::ACQUIRE);

		for (uint32_t i = 0; i < v_HoldCount; ++i)
			m_Lock.unlock();

		bool v_Signaled = false;
		if (!v_Deadline.isExpired()) {
#ifdef _WIN32
			const int64_t v_Ms = v_Deadline.remainingMilliseconds();
			const DWORD v_Timeout = (v_Ms <= 0) ? 0u
				: static_cast<DWORD>(v_Ms < 0xFFFFFFFELL ? v_Ms : 0xFFFFFFFEu);
			WaitOnAddress(m_Seq.data(), const_cast<uint32_t*>(&v_Seq), sizeof(uint32_t), v_Timeout);
#else
			// TODO: Linux futex with timeout
#endif
			v_Signaled = (m_Seq.load(Core::Atomics::MemoryOrder::ACQUIRE) != v_Seq);
		}

		m_Lock.lock();
		for (uint32_t i = 1; i < v_HoldCount; ++i)
			m_Lock.lock();

		return v_Signaled;
	}

	void Condition::signal() noexcept {
		m_Seq.fetchAdd(1u, Core::Atomics::MemoryOrder::ACQ_REL);
#ifdef _WIN32
		WakeByAddressSingle(m_Seq.data());
#else
		// TODO: Linux futex_wake
#endif
	}

	void Condition::signalAll() noexcept {
		m_Seq.fetchAdd(1u, Core::Atomics::MemoryOrder::ACQ_REL);
#ifdef _WIN32
		WakeByAddressAll(m_Seq.data());
#else
		// TODO: Linux futex_wake broadcast
#endif
	}

	// ─── Phaser ──────────────────────────────────────────────────────────────

	Phaser::Phaser(uint32_t v_Parties) noexcept {
		m_Registered = v_Parties;
	}

	Phaser::Phaser(Phaser& ro_Parent, uint32_t v_Parties) noexcept {
		m_Registered = v_Parties;
		m_pParent    = &ro_Parent;
		if (v_Parties > 0)
			ro_Parent.register_();
	}

	// Called with m_Lock held; updates phase state, then wakes waiters and
	// propagates to parent AFTER releasing the lock.
	void Phaser::doAdvance() noexcept {
		const uint32_t v_OldPhase = m_Phase.load(Core::Atomics::MemoryOrder::RELAXED);
		const bool     v_Terminate = onAdvance(v_OldPhase, m_Registered);

		if (v_Terminate || m_Registered == 0) {
			m_Terminated.store(1u, Core::Atomics::MemoryOrder::RELEASE);
		} else {
			m_Phase.increment(Core::Atomics::MemoryOrder::RELEASE);
			m_Arrived = 0;
		}
		m_Gate.fetchAdd(1u, Core::Atomics::MemoryOrder::ACQ_REL);
		m_Lock.unlock();

#ifdef _WIN32
		WakeByAddressAll(m_Gate.data());
#else
		// TODO: Linux futex_wake broadcast
#endif

		if (m_pParent) {
			if (m_Terminated.load(Core::Atomics::MemoryOrder::ACQUIRE))
				m_pParent->arriveAndDeregister();
			else
				m_pParent->arrive();
		}
	}

	bool Phaser::onAdvance(uint32_t /*v_Phase*/, uint32_t v_RegisteredParties) noexcept {
		return v_RegisteredParties == 0;
	}

	uint32_t Phaser::register_() noexcept {
		m_Lock.lock();
		++m_Registered;
		const uint32_t v_Phase = m_Phase.load(Core::Atomics::MemoryOrder::RELAXED);
		m_Lock.unlock();
		return v_Phase;
	}

	uint32_t Phaser::bulkRegister(uint32_t v_Parties) noexcept {
		m_Lock.lock();
		m_Registered += v_Parties;
		const uint32_t v_Phase = m_Phase.load(Core::Atomics::MemoryOrder::RELAXED);
		m_Lock.unlock();
		return v_Phase;
	}

	uint32_t Phaser::arrive() noexcept {
		m_Lock.lock();
		if (m_Terminated.load(Core::Atomics::MemoryOrder::ACQUIRE)) {
			const uint32_t v_Phase = m_Phase.load(Core::Atomics::MemoryOrder::RELAXED);
			m_Lock.unlock();
			return v_Phase;
		}
		const uint32_t v_Phase = m_Phase.load(Core::Atomics::MemoryOrder::RELAXED);
		++m_Arrived;
		if (m_Arrived >= m_Registered) {
			doAdvance(); // releases lock internally
		} else {
			m_Lock.unlock();
		}
		return v_Phase;
	}

	uint32_t Phaser::arriveAndDeregister() noexcept {
		m_Lock.lock();
		if (m_Terminated.load(Core::Atomics::MemoryOrder::ACQUIRE)) {
			const uint32_t v_Phase = m_Phase.load(Core::Atomics::MemoryOrder::RELAXED);
			m_Lock.unlock();
			return v_Phase;
		}
		const uint32_t v_Phase       = m_Phase.load(Core::Atomics::MemoryOrder::RELAXED);
		const uint32_t v_OldRegistered = m_Registered;
		if (m_Registered > 0) --m_Registered;
		++m_Arrived;
		if (m_Arrived >= v_OldRegistered) {
			doAdvance(); // releases lock internally
		} else {
			m_Lock.unlock();
		}
		return v_Phase;
	}

	uint32_t Phaser::arriveAndAwaitAdvance() noexcept {
		const uint32_t v_Phase = arrive();
		return awaitAdvance(v_Phase);
	}

	uint32_t Phaser::awaitAdvance(uint32_t v_Phase) noexcept {
		for (int i = 0; i < CORIUM_SPIN_COUNT; ++i) {
			const uint32_t v_Cur = m_Phase.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (v_Cur != v_Phase || m_Terminated.load(Core::Atomics::MemoryOrder::ACQUIRE)) return v_Cur;
			Intrinsic::Pause();
		}
		while (true) {
			const uint32_t v_Cur  = m_Phase.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (v_Cur != v_Phase || m_Terminated.load(Core::Atomics::MemoryOrder::ACQUIRE)) return v_Cur;
			const uint32_t v_Gate = m_Gate.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (m_Phase.load(Core::Atomics::MemoryOrder::ACQUIRE) != v_Phase) return m_Phase.load(Core::Atomics::MemoryOrder::ACQUIRE);
#ifdef _WIN32
			WaitOnAddress(m_Gate.data(), const_cast<uint32_t*>(&v_Gate), sizeof(uint32_t), INFINITE);
#else
			// TODO: Linux futex
#endif
		}
	}

	bool Phaser::awaitAdvance(uint32_t v_Phase, Chrono::Instant v_Deadline) noexcept {
		for (int i = 0; i < CORIUM_SPIN_COUNT; ++i) {
			const uint32_t v_Cur = m_Phase.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (v_Cur != v_Phase || m_Terminated.load(Core::Atomics::MemoryOrder::ACQUIRE)) return true;
			Intrinsic::Pause();
		}
		while (true) {
			const uint32_t v_Cur = m_Phase.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (v_Cur != v_Phase || m_Terminated.load(Core::Atomics::MemoryOrder::ACQUIRE)) return true;
			if (v_Deadline.isExpired()) return false;
			const uint32_t v_Gate = m_Gate.load(Core::Atomics::MemoryOrder::ACQUIRE);
			if (m_Phase.load(Core::Atomics::MemoryOrder::ACQUIRE) != v_Phase) return true;
#ifdef _WIN32
			const int64_t v_Ms      = v_Deadline.remainingMilliseconds();
			const DWORD   v_Timeout = (v_Ms <= 0) ? 0u
				: static_cast<DWORD>(v_Ms < 0xFFFFFFFELL ? v_Ms : 0xFFFFFFFEu);
			WaitOnAddress(m_Gate.data(), const_cast<uint32_t*>(&v_Gate), sizeof(uint32_t), v_Timeout);
#else
			// TODO: Linux futex with timeout
			return false;
#endif
		}
	}

	void Phaser::forceTermination() noexcept {
		m_Lock.lock();
		if (m_Terminated.load(Core::Atomics::MemoryOrder::ACQUIRE)) {
			m_Lock.unlock();
			return;
		}
		m_Terminated.store(1u, Core::Atomics::MemoryOrder::RELEASE);
		m_Gate.fetchAdd(1u, Core::Atomics::MemoryOrder::ACQ_REL);
		m_Lock.unlock();
#ifdef _WIN32
		WakeByAddressAll(m_Gate.data());
#else
		// TODO: Linux futex_wake broadcast
#endif
		if (m_pParent) m_pParent->arriveAndDeregister();
	}

	uint32_t Phaser::getPhase() const noexcept {
		return m_Phase.load(Core::Atomics::MemoryOrder::ACQUIRE);
	}

	uint32_t Phaser::getRegisteredParties() const noexcept {
		m_Lock.lock();
		const uint32_t v_Registered = m_Registered;
		m_Lock.unlock();
		return v_Registered;
	}

	uint32_t Phaser::getArrivedParties() const noexcept {
		m_Lock.lock();
		const uint32_t v_Arrived = m_Arrived;
		m_Lock.unlock();
		return v_Arrived;
	}

	uint32_t Phaser::getUnarrivedParties() const noexcept {
		m_Lock.lock();
		const uint32_t v_Unarrived = (m_Registered > m_Arrived) ? m_Registered - m_Arrived : 0u;
		m_Lock.unlock();
		return v_Unarrived;
	}

	bool Phaser::isTerminated() const noexcept {
		return m_Terminated.load(Core::Atomics::MemoryOrder::ACQUIRE) != 0u;
	}
}