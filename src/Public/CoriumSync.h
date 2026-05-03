#pragma once
#include "AtomicVariable.h"
#include "ThreadUtils.h"

// Some of my favorites from java.util.concurrent now in C++!
// I'm lazy to fix some bugs, once that's done I'll add the TODOS :)

namespace Corium::Runtime::Sync {

	class CORIUM_RUNTIME_API CountDownLatch final {
		// We are using ParkHandle here since all the NativeThread wait-on-address and wake-on-address 
		// implemenations are defined on ParkHandle
		Core::ParkHandle m_Counter;

	public:
		explicit CountDownLatch(uint32_t v_Count);

		CountDownLatch(const CountDownLatch&) = delete;
		CountDownLatch& operator=(const CountDownLatch&) = delete;

		// TODO CountDownLatch Move ctor and operator

		void countDown();
		void await();
		void await(long duration); // TODO
		uint32_t getCount() const;
	};

	class CORIUM_RUNTIME_API CyclicBarrier final {
		Core::ParkHandle m_Generation;
		const uint32_t m_Parties;
		Core::Atomic::AtomicValue32<uint32_t> m_Remaining;
		Core::Atomic::AtomicValue32<bool> m_IsBroken;
	public:
		explicit CyclicBarrier(uint32_t v_Parties);

		CyclicBarrier(const CyclicBarrier&) = delete;
		CyclicBarrier& operator=(const CyclicBarrier&) = delete;

		uint32_t await();
		bool await(long v_Duration); // TODO
		void reset();
		bool isBroken() const;
		uint32_t getParties() const;
		uint32_t getNumberWaiting() const;
	};

	class CORIUM_RUNTIME_API Semaphore final {
		// We are using ParkHandle here since all the NativeThread wait-on-address and wake-on-address
		// implemenations are defined on ParkHandle
		Core::ParkHandle m_Permits;
	public:
		explicit Semaphore(uint32_t v_Permits);

		Semaphore(const Semaphore&) = delete;
		Semaphore& operator=(const Semaphore&) = delete;

		void acquire();
		void acquire(uint32_t v_Permits);
		bool tryAcquire();
		bool tryAcquire(long v_Timeout); // TODO
		void release();
		void release(uint32_t v_Permits);
		uint32_t availablePermits() const;
	};
}
