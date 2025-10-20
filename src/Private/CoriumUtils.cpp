#include "Corium.h"
#include "CoriumUtils.h"

namespace Corium::Utils {
	uint64_t splitMix64(uint64_t v_Input) noexcept {
		v_Input += 0x9e3779b97f4a7c15ULL;
		v_Input = (v_Input ^ (v_Input >> 30)) * 0xbf58476d1ce4e5b9ULL;
		v_Input = (v_Input ^ (v_Input >> 27)) * 0x94d049bb133111ebULL;
		return v_Input ^ (v_Input >> 31);
	}

    uint64_t rdtsc() noexcept {
#if defined(_MSC_VER)
        return __rdtsc();
#elif defined(__i386__)
        uint64_t x;
        __asm__ volatile ("rdtsc" : "=A" (x));
        return x;
#elif defined(__x86_64__)
        uint32_t hi, lo;
        __asm__ volatile ("rdtsc" : "=a"(lo), "=d"(hi));
        return ((uint64_t) hi << 32) | lo;
#else
#   error "rdtsc not supported on this architecture"
#endif
    }

    uint64_t hash() {
        thread_local uint64_t state = rdtsc() * reinterpret_cast<uintptr_t>(&state);
        return splitMix64(state);
    }
}