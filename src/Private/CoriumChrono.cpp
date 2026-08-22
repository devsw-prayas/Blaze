#include "Corium.h"

#define ALLOW_SYSCALL
#include "CoriumSyscalls.h"
#include "CoriumChrono.h"

namespace Corium::Core::Chrono {

#ifdef _WIN32
	namespace {
		// QPC frequency cached on first use — constant per-boot, safe without a lock.
		static int64_t s_QpcFrequency = 0;

		CORIUM_FORCEINLINE int64_t queryNow() noexcept {
			if (s_QpcFrequency == 0) {
				LARGE_INTEGER frequency;
				QueryPerformanceFrequency(&frequency);
				s_QpcFrequency = frequency.QuadPart;
			}
			LARGE_INTEGER counter;
			QueryPerformanceCounter(&counter);
			// Split into seconds + sub-second remainder to avoid int64 overflow.
			const int64_t seconds = counter.QuadPart / s_QpcFrequency;
			const int64_t remainder  = counter.QuadPart % s_QpcFrequency;
			return seconds * 1'000'000'000LL + (remainder * 1'000'000'000LL / s_QpcFrequency);
		}
	}
#endif

	Instant MonotonicClock::now() noexcept {
#ifdef _WIN32
		return Instant{ queryNow() };
#else
		// TODO: Linux clock_gettime(CLOCK_MONOTONIC)
		return Instant{ 0 };
#endif
	}

	Instant until(Interval v_Interval) noexcept {
		return Instant{ MonotonicClock::now().m_Nanoseconds + v_Interval.m_Nanoseconds };
	}

	bool Instant::isExpired() const noexcept {
		return MonotonicClock::now().m_Nanoseconds >= m_Nanoseconds;
	}

	Interval Instant::remaining() const noexcept {
		const int64_t delta = m_Nanoseconds - MonotonicClock::now().m_Nanoseconds;
		return Interval{ delta > 0 ? delta : 0LL };
	}

	int64_t Instant::remainingMilliseconds() const noexcept {
		return remaining().toMilliseconds();
	}
}
