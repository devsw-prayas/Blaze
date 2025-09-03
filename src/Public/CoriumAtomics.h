#pragma once

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#elif defined(__linux__)
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <sched.h>
#endif

#if defined(_WIN32)
#ifndef SEQUENTIAL_FENCE
#define SEQUENTIAL_FENCE  _ReadWriteBarrier(); _mm_mfence();
#endif

#elif defined(__linux__)
#ifndef SEQUENTIAL_FENCE
#define SEQUENTIAL_FENCE __atomic_thread_fence(std::memory_order_seq_cst); 
#endif

#endif


#if defined(_WIN32)
#ifndef READ_FENCE
#define READ_FENCE  _ReadBarrier(); _mm_lfence();
#endif

#elif defined(__linux__)
#ifndef READ_FENCE
#define READ_FENCE __atomic_thread_fence(std::memory_order_acquire); 
#endif

#endif

#if defined(_WIN32)
#ifndef WRITE_FENCE
#define WRITE_FENCE  _WriteBarrier(); _mm_sfence();
#endif

#elif defined(__linux__)
#ifndef WRITE_FENCE
#define WRITE_FENCE __atomic_thread_fence(std::memory_order_release); 
#endif

#endif


namespace Corium::Atomics {

	template<typename  T>
	class Atomic final {
		
	};

	template<typename T, typename A, size_t Alignment>
	class EnhancedAtomic final {
		using Item = T;
		using Allocator = A;
	};
}
