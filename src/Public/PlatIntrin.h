/*
* Copyright (c) 2025 StormWeaver
*
* This file is part of the Corium Multithreading API
*
* Licensed under the MIT License. You may obtain a copy of the License at
* https://opensource.org/licenses/MIT
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND...
*/
#pragma once
#include <intrin.h>

// I'm not touching this file until something fucking breaks like 30,000 lines later
// fuck u future me
// Please don't modify this file, too many things gonna break too fast

namespace Corium::Intrinsic {
#define CORIUM_MEMORY_ORDER_RELAXED   0
#define CORIUM_MEMORY_ORDER_CONSUME   1  // treated as ACQUIRE on MSVC
#define CORIUM_MEMORY_ORDER_ACQUIRE   2
#define CORIUM_MEMORY_ORDER_RELEASE   3
#define CORIUM_MEMORY_ORDER_ACQ_REL   4
#define CORIUM_MEMORY_ORDER_SEQ_CST   5

	// Don't want some random compiler screaming cause some constant vanished in linux
#if !CORIUM_COMPILER_CLANG && !CORIUM_COMPILER_GCC
#define __ATOMIC_RELAXED 0
#define __ATOMIC_CONSUME 0
#define __ATOMIC_RELEASE 0
#define __ATOMIC_ACQUIRE 0
#define __ATOMIC_ACQ_REL 0
#define __ATOMIC_SEQ_CST 0
#endif

	CORIUM_FORCEINLINE CORIUM_RUNTIME_API  void Pause() {
		_mm_pause(); // Hardware pause
	}

	// Section containing hardware fences.

	// FullFence
	//
	// Establishes a global hardware ordering point.
	//
	// Guarantees that all loads and stores issued before this point become
	// visible before any loads or stores issued after it.
	//
	// This is a heavyweight synchronization primitive intended only for
	// global phase transitions, device boundaries, or shutdown paths.

	CORIUM_FORCEINLINE CORIUM_RUNTIME_API void FullFence() {
#if CORIUM_COMPILER_MSVC
		::_mm_mfence();
#elif CORIUM_COMPILER_GCC
#if defined(__has_builtin)
#if __has_builtin(__atomic_thread_fence)
		__atomic_thread_fence(__ATOMIC_SEQ_CST);
#else
		CORIUM_STATIC_ASSERT(__has_builtin(__atomic_thread_fence), "__atomic_thread_fence not supported by this compiler");
#endif
#else
		CORIUM_STATIC_ASSERT(false, "__has_builtin not available");
#endif
#else

		CORIUM_STATIC_ASSERT(false, "Unsupported full fence in this environment");
		CORIUM_UNREACHABLE();
#endif
	}

	// LoadFence
	//
	// Establishes ordering for load operations.
	//
	// Prevents later loads from being observed before earlier loads.
	// Does not impose ordering on stores.
	//
	// Intended for explicit consumption of published data.

	CORIUM_FORCEINLINE	CORIUM_RUNTIME_API void LoadFence() {
#if CORIUM_COMPILER_MSVC
		::_mm_lfence();
#elif CORIUM_COMPILER_GCC
#if defined(__has_builtin)
#if __has_builtin(__atomic_thread_fence)
		__atomic_thread_fence(__ATOMIC_ACQUIRE);
#else
		CORIUM_STATIC_ASSERT(__has_builtin(__atomic_thread_fence), "__atomic_thread_fence not supported by this compiler");
#endif
#else
		CORIUM_STATIC_ASSERT(false, "__has_builtin not available");
#endif
#else
		CORIUM_STATIC_ASSERT(false, "Unsupported load fence in this environment");
		CORIUM_UNREACHABLE();
#endif
	}

	// StoreFence
	//
	// Establishes ordering for store operations.
	//
	// Ensures that all prior stores are committed before subsequent stores.
	// Does not impose ordering on loads.
	//
	// Commonly used when publishing data followed by a visibility flag.

	CORIUM_FORCEINLINE CORIUM_RUNTIME_API void StoreFence() {
#if CORIUM_COMPILER_MSVC
		::_mm_sfence();
#elif  CORIUM_COMPILER_GCC
#if defined(__has_builtin)
#if __has_builtin(__atomic_thread_fence)
		__atomic_thread_fence(__ATOMIC_RELEASE);
#else
		CORIUM_STATIC_ASSERT(__has_builtin(__atomic_thread_fence), "__atomic_thread_fence not supported by this compiler");
#endif
#else
		CORIUM_STATIC_ASSERT(false, "__has_builtin not available");
#endif
#else
		CORIUM_STATIC_ASSERT(false, "Unsupported store fence in this environment");
		CORIUM_UNREACHABLE();
#endif
	}

	// (Why is this even necessary?) Compiler hint fences

	// RWCompileBarrier
	//
	// Compiler-only barrier for both loads and stores.
	//
	// Prevents the compiler from reordering memory operations across
	// this point, without emitting any CPU instructions.
	//
	// Does NOT provide inter-thread synchronization.

	CORIUM_FORCEINLINE CORIUM_RUNTIME_API void RWCompileBarrier() {
#if CORIUM_COMPILER_MSVC
		_ReadWriteBarrier();
#elif CORIUM_COMPILER_GCC
#if defined(__has_builtin)
#if __has_builtin(__atomic_signal_fence)
		__atomic_signal_fence(__ATOMIC_SEQ_CST);
# else
		CORIUM_STATIC_ASSERT(__has_builtin(__atomic_signal_fence), "__atomic_signal_fence not supported by this compiler");
#endif
#else
		CORIUM_STATIC_ASSERT(false, "__has_builtin not available");
#endif
#else
		CORIUM_STATIC_ASSERT(false, "Unsupported read write barrier in this environment");
		CORIUM_UNREACHABLE();
#endif
	}

	// RCompileBarrier
	//
	// Compiler-only barrier for load operations.
	//
	// Prevents reordering of reads across this point while allowing
	// stores to move freely.
	//
	// Intended for rare, read-only ordering constraints.

	CORIUM_FORCEINLINE CORIUM_RUNTIME_API void RCompileBarrier() {
#if CORIUM_COMPILER_MSVC
		_ReadBarrier();
#elif CORIUM_COMPILER_GCC
#if defined(__has_builtin)
#if __has_builtin(__atomic_signal_fence)
		__atomic_signal_fence(__ATOMIC_ACQUIRE);
#else
		CORIUM_STATIC_ASSERT(__has_builtin(__atomic_signal_fence), "__atomic_signal_fence not supported by this compiler");
#endif
#else
		CORIUM_STATIC_ASSERT(false, "__has_builtin not available");
#endif
#else
		CORIUM_STATIC_ASSERT(false, "Unsupported read barrier in this environment");
		CORIUM_UNREACHABLE();
#endif
	}

	// WCompileBarrier
	//
	// Compiler-only barrier for store operations.
	//
	// Prevents reordering of writes across this point while allowing
	// loads to move freely.
	//
	// Commonly used before publishing shared state.

	CORIUM_FORCEINLINE CORIUM_RUNTIME_API void WCompileBarrier() {
#if CORIUM_COMPILER_MSVC
		_WriteBarrier();
#elif CORIUM_COMPILER_GCC
#if defined(__has_builtin)
#if __has_builtin(__atomic_signal_fence)
		__atomic_signal_fence(__ATOMIC_RELEASE);
#else
		CORIUM_STATIC_ASSERT(false, "__atomic_signal_fence not supported by this compiler");
#endif
#else
		CORIUM_STATIC_ASSERT(false, "__has_builtin not available");
#endif
#else
		CORIUM_STATIC_ASSERT(false, "Unsupported write barrier in this environment");
		CORIUM_UNREACHABLE();
#endif
	}

	template<typename T>
	struct ValidAtomicParameter final {
		using Type = T;
		CORIUM_STATIC_ASSERT(
			std::is_trivially_copyable_v<Type>,
			"Unsupported atomic nature: type must be trivially copyable."
		);

		CORIUM_STATIC_ASSERT(
			!std::is_const_v<Type>,
			"Unsupported atomic nature: atomic type must not be const-qualified."
		);

		CORIUM_STATIC_ASSERT(
			!std::is_volatile_v<Type>,
			"Unsupported atomic nature: atomic type must not be volatile-qualified."
		);
	};

	// Moving on to a wall of macros, fuck!!

	// AtomicLoad — reads the value at the target address as a single indivisible operation; does not modify it.

#ifndef AtomicLoad_Relaxed
#if CORIUM_COMPILER_MSVC
#define AtomicLoad_Relaxed(p_Ptr) \
    (*reinterpret_cast<volatile std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_load_n)
#define AtomicLoad_Relaxed(p_Ptr) \
	(__atomic_load_n((p_Ptr), __ATOMIC_RELAXED))
#else
#error "Missing builtin GNU intrinsic: __atomic_load_n"
#endif

#else
#error "Unsupported compiler for atomic load"
#endif

#endif

#ifndef AtomicLoad_Acquire
#if CORIUM_COMPILER_MSVC
#define AtomicLoad_Acquire(p_Ptr) \
    (*reinterpret_cast<volatile std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr))

#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC

#if __has_builtin(__atomic_load_n)
#define AtomicLoad_Acquire(p_Ptr) \
    (__atomic_load_n((p_Ptr), __ATOMIC_ACQUIRE))
#else
#error "Missing builtin GNU intrinsic: __atomic_load_n"
#endif

#else
#error "Unsupported compiler for atomic load (acquire)"
#endif

#endif

#ifndef AtomicLoad_Consume
#if CORIUM_COMPILER_MSVC
	/* CONSUME collapses to ACQUIRE */
#define AtomicLoad_Consume(p_Ptr) \
    (*reinterpret_cast<volatile std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_load_n)
	/* CONSUME collapses to ACQUIRE */
#define AtomicLoad_Consume(p_Ptr)                                      \
    (__atomic_load_n((p_Ptr), __ATOMIC_CONSUME))
#else
#error "Missing builtin GNU intrinsic: __atomic_load_n"
#endif

#else
#error "Unsupported compiler for atomic load (consume)"
#endif

#endif

#ifndef AtomicLoad_SeqCst
#if CORIUM_COMPILER_MSVC
	/*
	 * MSVC x64 has no standalone seq_cst load.
	 * Volatile load + full fence is the strongest representable form.
	 */
#define AtomicLoad_SeqCst(p_Ptr)                                       \
    ([&]() {                                                           \
        auto _v = *reinterpret_cast<volatile std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr);                                                      \
        _mm_mfence();                                                   \
        return _v;                                                      \
    }())
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_load_n)
#define AtomicLoad_SeqCst(p_Ptr) \
    (__atomic_load_n((p_Ptr), __ATOMIC_SEQ_CST))
#else
#error "Missing builtin GNU intrinsic: __atomic_load_n"
#endif

#else
#error "Unsupported compiler for atomic load (seq_cst)"
#endif

#endif

	// AtomicStore — writes a value to the target address as a single indivisible update visible to all threads.

#ifndef AtomicStore_Relaxed
#if CORIUM_COMPILER_MSVC

#define AtomicStore_Relaxed(p_Ptr, v_Value)		  \
		(*reinterpret_cast<volatile std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr) = (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_store_n)
#define AtomicStore_Relaxed(p_Ptr, v_Value)			 \
	__atomic_store_n(p_Ptr, v_Value, __ATOMIC_RELAXED)
#else
#error "Missing builtin GNU intrinsic: __atomic_store_n"
#endif

#else
#error "Unsupported compiler for atomic store (relaxed)"
#endif

#endif

#ifndef AtomicStore_Release
#if CORIUM_COMPILER_MSVC

#define AtomicStore_Release(p_Ptr, v_Value)		  \
	(*reinterpret_cast<volatile std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr) = (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_store_n)
#define AtomicStore_Release(p_Ptr, v_Value)			 \
	__atomic_store_n(p_Ptr, v_Value, __ATOMIC_RELEASE)
#else
#error "Missing builtin GNU intrinsic: __atomic_store_n"
#endif

#else
#error "Unsupported compiler for atomic store (release)"
#endif

#endif

#ifndef AtomicStore_SeqCst
#if CORIUM_COMPILER_MSVC
/*
* MSVC x64 has no standalone seq_cst store.
* Volatile store + full fence is the strongest representable form.
*/
#define AtomicStore_SeqCst(p_Ptr, v_Value)		  \
	([&]() {                                                           \
		*reinterpret_cast<volatile std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr) = v_Value;			 \
		_mm_mfence();                                                   \
		}())
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_store_n)
#define AtomicStore_SeqCst(p_Ptr, v_Value)			 \
	__atomic_store_n(p_Ptr, v_Value, __ATOMIC_SEQ_CST)
#else
#error "Missing builtin GNU intrinsic: __atomic_store_n"
#endif

#else
#error "Unsupported compiler for atomic store (seq cst)"
#endif

#endif

	// AtomicExchange — replaces the value at the target address, returning the previous value.

#ifndef AtomicExchange32_Relaxed
#if CORIUM_COMPILER_MSVC
#define AtomicExchange32_Relaxed(p_Ptr, v_Value) \
		_InterlockedExchange((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_exchange_n)
#define AtomicExchange32_Relaxed(p_Ptr, v_Value) \
		__atomic_exchange_n((p_Ptr), (v_Value), __ATOMIC_RELAXED)
#else
#error "Missing builtin GNU intrinisc: __atomic_exchange_n"
#endif

#else
#error "Unsupported compiler for atomic exchange (relaxed)"
#endif
#endif

#ifndef AtomicExchange64_Relaxed
#if CORIUM_COMPILER_MSVC
#define AtomicExchange64_Relaxed(p_Ptr, v_Value) \
		_InterlockedExchange64((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_exchange_n)
#define AtomicExchange64_Relaxed(p_Ptr, v_Value) \
		__atomic_exchange_n((p_Ptr), (v_Value), __ATOMIC_RELAXED)
#else
#error "Missing builtin GNU intrinisc: __atomic_exchange_n"
#endif

#else
#error "Unsupported compiler for atomic exchange (relaxed)"
#endif
#endif

#ifndef AtomicExchange32_AcqRel
#if CORIUM_COMPILER_MSVC
#define AtomicExchange32_AcqRel(p_Ptr, v_Value) \
		_InterlockedExchange((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_exchange_n)
#define AtomicExchange32_AcqRel(p_Ptr, v_Value) \
		__atomic_exchange_n((p_Ptr), (v_Value), __ATOMIC_ACQ_REL)
#else
#error "Missing builtin GNU intrinisc: __atomic_exchange_n"
#endif

#else
#error "Unsupported compiler for atomic exchange (AcqRel)"
#endif
#endif

#ifndef AtomicExchange64_AcqRel
#if CORIUM_COMPILER_MSVC
#define AtomicExchange64_AcqRel(p_Ptr, v_Value) \
		_InterlockedExchange64((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_exchange_n)
#define AtomicExchange64_AcqRel(p_Ptr, v_Value) \
		__atomic_exchange_n((p_Ptr), (v_Value), __ATOMIC_ACQ_REL)
#else
#error "Missing builtin GNU intrinisc: __atomic_exchange_n"
#endif

#else
#error "Unsupported compiler for atomic exchange (AcqRel)"
#endif
#endif

#ifndef AtomicExchange32_SeqCst
#if CORIUM_COMPILER_MSVC
#define AtomicExchange32_SeqCst(p_Ptr, v_Value) \
		_InterlockedExchange((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_exchange_n)
#define AtomicExchange32_SeqCst(p_Ptr, v_Value) \
		__atomic_exchange_n((p_Ptr), (v_Value), __ATOMIC_SEQ_CST)
#else
#error "Missing builtin GNU intrinisc: __atomic_exchange_n"
#endif

#else
#error "Unsupported compiler for atomic exchange (SeqCst)"
#endif
#endif

#ifndef AtomicExchange64_SeqCst
#if CORIUM_COMPILER_MSVC
#define AtomicExchange64_SeqCst(p_Ptr, v_Value) \
		_InterlockedExchange64((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_exchange_n)
#define AtomicExchange64_SeqCst(p_Ptr, v_Value) \
		__atomic_exchange_n((p_Ptr), (v_Value), __ATOMIC_SEQ_CST)
#else
#error "Missing builtin GNU intrinisc: __atomic_exchange_n"
#endif

#else
#error "Unsupported compiler for atomic exchange (Seq Cst)"
#endif
#endif

	// AtomicCompareExchange — replaces the value at the target address with desired if it equals expected; returns whether it succeeded, and updates expected on failure.

#ifndef AtomicCompareExchange32
#if CORIUM_COMPILER_MSVC
#define AtomicCompareExchange32(p_Ptr, expected, desired, weak, success_memOrder, failure_memOrder) \
        _InterlockedCompareExchange((p_Ptr), (desired), *(expected))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_compare_exchange_n)
#define AtomicCompareExchange32(p_Ptr, expected, desired, weak, success_memOrder, failure_memOrder) \
        __atomic_compare_exchange_n((p_Ptr), (expected), (desired), (weak), (success_memOrder), (failure_memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_compare_exchange_n"
#endif
#else
#error "Unsupported compiler for atomic compare exchange"
#endif
#endif

// 1. Success: RELAXED | Failure: RELAXED
#define AtomicCompareExchange32_Relaxed_Relaxed(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_RELAXED, __ATOMIC_RELAXED)

// 2. Success: RELAXED | Failure: ACQUIRE
#define AtomicCompareExchange32_Relaxed_Acquire(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_RELAXED, __ATOMIC_ACQUIRE)

// 3. Success: RELAXED | Failure: SEQ_CST
#define AtomicCompareExchange32_Relaxed_SeqCst(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_RELAXED, __ATOMIC_SEQ_CST)

// 4. Success: ACQUIRE | Failure: RELAXED
#define AtomicCompareExchange32_Acquire_Relaxed(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)

// 5. Success: ACQUIRE | Failure: ACQUIRE
#define AtomicCompareExchange32_Acquire_Acquire(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)

// 6. Success: ACQUIRE | Failure: SEQ_CST
#define AtomicCompareExchange32_Acquire_SeqCst(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_ACQUIRE, __ATOMIC_SEQ_CST)

// 7. Success: RELEASE | Failure: RELAXED
#define AtomicCompareExchange32_Release_Relaxed(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_RELEASE, __ATOMIC_RELAXED)

// 8. Success: RELEASE | Failure: ACQUIRE
#define AtomicCompareExchange32_Release_Acquire(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)

// 9. Success: RELEASE | Failure: SEQ_CST
#define AtomicCompareExchange32_Release_SeqCst(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_RELEASE, __ATOMIC_SEQ_CST)

// 10. Success: ACQ_REL | Failure: RELAXED
#define AtomicCompareExchange32_AcqRel_Relaxed(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)

// 11. Success: ACQ_REL | Failure: ACQUIRE
#define AtomicCompareExchange32_AcqRel_Acquire(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)

// 12. Success: ACQ_REL | Failure: SEQ_CST
#define AtomicCompareExchange32_AcqRel_SeqCst(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_ACQ_REL, __ATOMIC_SEQ_CST)

// 13. Success: SEQ_CST | Failure: RELAXED
#define AtomicCompareExchange32_SeqCst_Relaxed(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_SEQ_CST, __ATOMIC_RELAXED)

// 14. Success: SEQ_CST | Failure: ACQUIRE
#define AtomicCompareExchange32_SeqCst_Acquire(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE)

// 15. Success: SEQ_CST | Failure: SEQ_CST
#define AtomicCompareExchange32_SeqCst_SeqCst(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange32((p_Ptr), (expected), (desired), (weak), __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

#ifndef AtomicCompareExchange64
#if CORIUM_COMPILER_MSVC
#define AtomicCompareExchange64(p_Ptr, expected, desired, weak, success_memOrder, failure_memOrder) \
        _InterlockedCompareExchange64((p_Ptr), (desired), *(expected))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_compare_exchange_n)
#define AtomicCompareExchange64(p_Ptr, expected, desired, weak, success_memOrder, failure_memOrder) \
        __atomic_compare_exchange_n((p_Ptr), (expected), (desired), (weak), (success_memOrder), (failure_memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_compare_exchange_n"
#endif
#else
#error "Unsupported compiler for atomic compare exchange"
#endif
#endif

// 1. Success: RELAXED | Failure: RELAXED
#define AtomicCompareExchange64_Relaxed_Relaxed(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_RELAXED, __ATOMIC_RELAXED)

// 2. Success: RELAXED | Failure: ACQUIRE
#define AtomicCompareExchange64_Relaxed_Acquire(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_RELAXED, __ATOMIC_ACQUIRE)

// 3. Success: RELAXED | Failure: SEQ_CST
#define AtomicCompareExchange64_Relaxed_SeqCst(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_RELAXED, __ATOMIC_SEQ_CST)

// 4. Success: ACQUIRE | Failure: RELAXED
#define AtomicCompareExchange64_Acquire_Relaxed(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_ACQUIRE, __ATOMIC_RELAXED)

// 5. Success: ACQUIRE | Failure: ACQUIRE
#define AtomicCompareExchange64_Acquire_Acquire(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_ACQUIRE, __ATOMIC_ACQUIRE)

// 6. Success: ACQUIRE | Failure: SEQ_CST
#define AtomicCompareExchange64_Acquire_SeqCst(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_ACQUIRE, __ATOMIC_SEQ_CST)

// 7. Success: RELEASE | Failure: RELAXED
#define AtomicCompareExchange64_Release_Relaxed(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_RELEASE, __ATOMIC_RELAXED)

// 8. Success: RELEASE | Failure: ACQUIRE
#define AtomicCompareExchange64_Release_Acquire(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_RELEASE, __ATOMIC_ACQUIRE)

// 9. Success: RELEASE | Failure: SEQ_CST
#define AtomicCompareExchange64_Release_SeqCst(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_RELEASE, __ATOMIC_SEQ_CST)

// 10. Success: ACQ_REL | Failure: RELAXED
#define AtomicCompareExchange64_AcqRel_Relaxed(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_ACQ_REL, __ATOMIC_RELAXED)

// 11. Success: ACQ_REL | Failure: ACQUIRE
#define AtomicCompareExchange64_AcqRel_Acquire(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)

// 12. Success: ACQ_REL | Failure: SEQ_CST
#define AtomicCompareExchange64_AcqRel_SeqCst(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_ACQ_REL, __ATOMIC_SEQ_CST)

// 13. Success: SEQ_CST | Failure: RELAXED
#define AtomicCompareExchange64_SeqCst_Relaxed(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_SEQ_CST, __ATOMIC_RELAXED)

// 14. Success: SEQ_CST | Failure: ACQUIRE
#define AtomicCompareExchange64_SeqCst_Acquire(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_SEQ_CST, __ATOMIC_ACQUIRE)

// 15. Success: SEQ_CST | Failure: SEQ_CST
#define AtomicCompareExchange64_SeqCst_SeqCst(p_Ptr, expected, desired, weak) \
        AtomicCompareExchange64((p_Ptr), (expected), (desired), (weak), __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

	// AtomicFetchAdd — adds a value to the target address, returning the value stored prior to the addition.

#ifndef AtomicFetchAdd32
#if CORIUM_COMPILER_MSVC
#define AtomicFetchAdd32(p_Ptr, v_Value, memOrder) \
        _InterlockedExchangeAdd((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_fetch_add)
#define AtomicFetchAdd32(p_Ptr, v_Value, memOrder) \
        __atomic_fetch_add((p_Ptr), (v_Value), (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_fetch_add"
#endif
#else
#error "Unsupported compiler for atomic fetch add"
#endif
#endif

// 1. Success: RELAXED
#define AtomicFetchAdd32_Relaxed(p_Ptr, v_Value) \
        AtomicFetchAdd32((p_Ptr), (v_Value), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicFetchAdd32_Acquire(p_Ptr, v_Value) \
        AtomicFetchAdd32((p_Ptr), (v_Value), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicFetchAdd32_Release(p_Ptr, v_Value) \
        AtomicFetchAdd32((p_Ptr), (v_Value), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicFetchAdd32_AcqRel(p_Ptr, v_Value) \
        AtomicFetchAdd32((p_Ptr), (v_Value), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicFetchAdd32_SeqCst(p_Ptr, v_Value) \
        AtomicFetchAdd32((p_Ptr), (v_Value), __ATOMIC_SEQ_CST)

#ifndef AtomicFetchAdd64
#if CORIUM_COMPILER_MSVC
#define AtomicFetchAdd64(p_Ptr, v_Value, memOrder) \
        _InterlockedExchangeAdd64((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_fetch_add)
#define AtomicFetchAdd64(p_Ptr, v_Value, memOrder) \
        __atomic_fetch_add((p_Ptr), (v_Value), (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_fetch_add"
#endif
#else
#error "Unsupported compiler for atomic fetch add"
#endif
#endif

// 1. Success: RELAXED
#define AtomicFetchAdd64_Relaxed(p_Ptr, v_Value) \
        AtomicFetchAdd64((p_Ptr), (v_Value), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicFetchAdd64_Acquire(p_Ptr, v_Value) \
        AtomicFetchAdd64((p_Ptr), (v_Value), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicFetchAdd64_Release(p_Ptr, v_Value) \
        AtomicFetchAdd64((p_Ptr), (v_Value), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicFetchAdd64_AcqRel(p_Ptr, v_Value) \
        AtomicFetchAdd64((p_Ptr), (v_Value), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicFetchAdd64_SeqCst(p_Ptr, v_Value) \
        AtomicFetchAdd64((p_Ptr), (v_Value), __ATOMIC_SEQ_CST)

	// AtomicAddFetch — adds a value to the target address, returning the value stored after the addition.

#ifndef AtomicAddFetch32
#if CORIUM_COMPILER_MSVC
#define AtomicAddFetch32(p_Ptr, v_Value, memOrder) \
        _InterlockedExchangeAdd((p_Ptr), (v_Value)) + (v_Value)
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_add_fetch)
#define AtomicAddFetch32(p_Ptr, v_Value, memOrder) \
        __atomic_add_fetch((p_Ptr), (v_Value), (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_add_fetch"
#endif
#else
#error "Unsupported compiler for atomic add fetch"
#endif
#endif

// 1. Success: RELAXED
#define AtomicAddFetch32_Relaxed(p_Ptr, v_Value) \
        AtomicAddFetch32((p_Ptr), (v_Value), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicAddFetch32_Acquire(p_Ptr, v_Value) \
        AtomicAddFetch32((p_Ptr), (v_Value), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicAddFetch32_Release(p_Ptr, v_Value) \
        AtomicAddFetch32((p_Ptr), (v_Value), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicAddFetch32_AcqRel(p_Ptr, v_Value) \
        AtomicAddFetch32((p_Ptr), (v_Value), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicAddFetch32_SeqCst(p_Ptr, v_Value) \
        AtomicAddFetch32((p_Ptr), (v_Value), __ATOMIC_SEQ_CST)

#ifndef AtomicAddFetch64
#if CORIUM_COMPILER_MSVC
#define AtomicAddFetch64(p_Ptr, v_Value, memOrder) \
        _InterlockedExchangeAdd64((p_Ptr), (v_Value)) + (v_Value)
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_add_fetch)
#define AtomicAddFetch64(p_Ptr, v_Value, memOrder) \
        __atomic_add_fetch((p_Ptr), (v_Value), (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_add_fetch"
#endif
#else
#error "Unsupported compiler for atomic add fetch"
#endif
#endif

// 1. Success: RELAXED
#define AtomicAddFetch64_Relaxed(p_Ptr, v_Value) \
        AtomicAddFetch64((p_Ptr), (v_Value), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicAddFetch64_Acquire(p_Ptr, v_Value) \
        AtomicAddFetch64((p_Ptr), (v_Value), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicAddFetch64_Release(p_Ptr, v_Value) \
        AtomicAddFetch64((p_Ptr), (v_Value), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicAddFetch64_AcqRel(p_Ptr, v_Value) \
        AtomicAddFetch64((p_Ptr), (v_Value), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicAddFetch64_SeqCst(p_Ptr, v_Value) \
        AtomicAddFetch64((p_Ptr), (v_Value), __ATOMIC_SEQ_CST)

	// AtomicIncrement — atomically increments the value at the target address by one.

#ifndef AtomicIncrement32
#if CORIUM_COMPILER_MSVC
#define AtomicIncrement32(p_Ptr, memOrder) \
        _InterlockedIncrement((p_Ptr))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_fetch_add)
#define AtomicIncrement32(p_Ptr, memOrder) \
        __atomic_add_fetch((p_Ptr), 1, (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_add_fetch"
#endif
#else
#error "Unsupported compiler for atomic increment"
#endif
#endif

// 1. Success: RELAXED
#define AtomicIncrement32_Relaxed(p_Ptr) \
        AtomicIncrement32((p_Ptr), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicIncrement32_Acquire(p_Ptr) \
        AtomicIncrement32((p_Ptr), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicIncrement32_Release(p_Ptr) \
        AtomicIncrement32((p_Ptr), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicIncrement32_AcqRel(p_Ptr) \
        AtomicIncrement32((p_Ptr), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicIncrement32_SeqCst(p_Ptr) \
        AtomicIncrement32((p_Ptr), __ATOMIC_SEQ_CST)

#ifndef AtomicDecrement32
#if CORIUM_COMPILER_MSVC
#define AtomicDecrement32(p_Ptr, memOrder) \
        _InterlockedDecrement((p_Ptr))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_fetch_sub)
#define AtomicDecrement32(p_Ptr, memOrder) \
        __atomic_sub_fetch((p_Ptr), 1, (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_sub_fetch"
#endif
#else
#error "Unsupported compiler for atomic decrement"
#endif
#endif

// 1. Success: RELAXED
#define AtomicDecrement32_Relaxed(p_Ptr) \
        AtomicDecrement32((p_Ptr), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicDecrement32_Acquire(p_Ptr) \
        AtomicDecrement32((p_Ptr), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicDecrement32_Release(p_Ptr) \
        AtomicDecrement32((p_Ptr), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicDecrement32_AcqRel(p_Ptr) \
        AtomicDecrement32((p_Ptr), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicDecrement32_SeqCst(p_Ptr) \
        AtomicDecrement32((p_Ptr), __ATOMIC_SEQ_CST)

#ifndef AtomicIncrement64
#if CORIUM_COMPILER_MSVC
#define AtomicIncrement64(p_Ptr, memOrder) \
        _InterlockedIncrement64((p_Ptr))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_fetch_add)
#define AtomicIncrement64(p_Ptr, memOrder) \
        __atomic_add_fetch((p_Ptr), 1, (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_add_fetch"
#endif
#else
#error "Unsupported compiler for atomic increment"
#endif
#endif

// 1. Success: RELAXED
#define AtomicIncrement64_Relaxed(p_Ptr) \
        AtomicIncrement64((p_Ptr), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicIncrement64_Acquire(p_Ptr) \
        AtomicIncrement64((p_Ptr), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicIncrement64_Release(p_Ptr) \
        AtomicIncrement64((p_Ptr), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicIncrement64_AcqRel(p_Ptr) \
        AtomicIncrement64((p_Ptr), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicIncrement64_SeqCst(p_Ptr) \
        AtomicIncrement64((p_Ptr), __ATOMIC_SEQ_CST)

	// AtomicDecrement — atomically decrements the value at the target address by one.

#ifndef AtomicDecrement64
#if CORIUM_COMPILER_MSVC
#define AtomicDecrement64(p_Ptr, memOrder) \
        _InterlockedDecrement64((p_Ptr))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_fetch_sub)
#define AtomicDecrement64(p_Ptr, memOrder) \
        __atomic_sub_fetch((p_Ptr), 1, (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_sub_fetch"
#endif
#else
#error "Unsupported compiler for atomic decrement"
#endif
#endif

// 1. Success: RELAXED
#define AtomicDecrement64_Relaxed(p_Ptr) \
        AtomicDecrement64((p_Ptr), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicDecrement64_Acquire(p_Ptr) \
        AtomicDecrement64((p_Ptr), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicDecrement64_Release(p_Ptr) \
        AtomicDecrement64((p_Ptr), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicDecrement64_AcqRel(p_Ptr) \
        AtomicDecrement64((p_Ptr), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicDecrement64_SeqCst(p_Ptr) \
        AtomicDecrement64((p_Ptr), __ATOMIC_SEQ_CST)

	// AtomicFetchAnd — ANDs the stored value with the provided mask, returning the previous value.

#ifndef AtomicFetchAnd
#if CORIUM_COMPILER_MSVC
#define AtomicFetchAnd(p_Ptr, v_Value, memOrder) \
        _InterlockedAnd((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_fetch_and)
#define AtomicFetchAnd(p_Ptr, v_Value, memOrder) \
        __atomic_fetch_and((p_Ptr), (v_Value), (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_fetch_and"
#endif
#else
#error "Unsupported compiler for atomic fetch and"
#endif
#endif

// 1. Success: RELAXED
#define AtomicFetchAnd_Relaxed(p_Ptr, v_Value) \
        AtomicFetchAnd((p_Ptr), (v_Value), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicFetchAnd_Acquire(p_Ptr, v_Value) \
        AtomicFetchAnd((p_Ptr), (v_Value), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicFetchAnd_Release(p_Ptr, v_Value) \
        AtomicFetchAnd((p_Ptr), (v_Value), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicFetchAnd_AcqRel(p_Ptr, v_Value) \
        AtomicFetchAnd((p_Ptr), (v_Value), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicFetchAnd_SeqCst(p_Ptr, v_Value) \
        AtomicFetchAnd((p_Ptr), (v_Value), __ATOMIC_SEQ_CST)

	// AtomicFetchOr — ORs the stored value with the provided mask, returning the previous value.

#ifndef AtomicFetchOr
#if CORIUM_COMPILER_MSVC
#define AtomicFetchOr(p_Ptr, v_Value, memOrder) \
        _InterlockedOr((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_fetch_or)
#define AtomicFetchOr(p_Ptr, v_Value, memOrder) \
        __atomic_fetch_or((p_Ptr), (v_Value), (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_fetch_or"
#endif
#else
#error "Unsupported compiler for atomic fetch or"
#endif
#endif

// 1. Success: RELAXED
#define AtomicFetchOr_Relaxed(p_Ptr, v_Value) \
        AtomicFetchOr((p_Ptr), (v_Value), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicFetchOr_Acquire(p_Ptr, v_Value) \
        AtomicFetchOr((p_Ptr), (v_Value), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicFetchOr_Release(p_Ptr, v_Value) \
        AtomicFetchOr((p_Ptr), (v_Value), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicFetchOr_AcqRel(p_Ptr, v_Value) \
        AtomicFetchOr((p_Ptr), (v_Value), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicFetchOr_SeqCst(p_Ptr, v_Value) \
        AtomicFetchOr((p_Ptr), (v_Value), __ATOMIC_SEQ_CST)

	// AtomicFetchXor — XORs the stored value with the provided mask, returning the previous value.

#ifndef AtomicFetchXor
#if CORIUM_COMPILER_MSVC
#define AtomicFetchXor(p_Ptr, v_Value, memOrder) \
        _InterlockedXor((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_fetch_xor)
#define AtomicFetchXor(p_Ptr, v_Value, memOrder) \
        __atomic_fetch_xor((p_Ptr), (v_Value), (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_fetch_xor"
#endif
#else
#error "Unsupported compiler for atomic fetch xor"
#endif
#endif

// 1. Success: RELAXED
#define AtomicFetchXor_Relaxed(p_Ptr, v_Value) \
        AtomicFetchXor((p_Ptr), (v_Value), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicFetchXor_Acquire(p_Ptr, v_Value) \
        AtomicFetchXor((p_Ptr), (v_Value), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicFetchXor_Release(p_Ptr, v_Value) \
        AtomicFetchXor((p_Ptr), (v_Value), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicFetchXor_AcqRel(p_Ptr, v_Value) \
        AtomicFetchXor((p_Ptr), (v_Value), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicFetchXor_SeqCst(p_Ptr, v_Value) \
        AtomicFetchXor((p_Ptr), (v_Value), __ATOMIC_SEQ_CST)

	// AtomicFetchNand — NANDs the stored value with the provided mask, returning the previous value.

#ifndef AtomicFetchNand
#if CORIUM_COMPILER_MSVC
#define AtomicFetchNand(p_Ptr, v_Value, memOrder) \
        _InterlockedNand((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_fetch_nand)
#define AtomicFetchNand(p_Ptr, v_Value, memOrder) \
        __atomic_fetch_nand((p_Ptr), (v_Value), (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_fetch_nand"
#endif
#else
#error "Unsupported compiler for atomic fetch nand"
#endif
#endif

// 1. Success: RELAXED
#define AtomicFetchNand_Relaxed(p_Ptr, v_Value) \
        AtomicFetchNand((p_Ptr), (v_Value), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicFetchNand_Acquire(p_Ptr, v_Value) \
        AtomicFetchNand((p_Ptr), (v_Value), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicFetchNand_Release(p_Ptr, v_Value) \
        AtomicFetchNand((p_Ptr), (v_Value), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicFetchNand_AcqRel(p_Ptr, v_Value) \
        AtomicFetchNand((p_Ptr), (v_Value), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicFetchNand_SeqCst(p_Ptr, v_Value) \
        AtomicFetchNand((p_Ptr), (v_Value), __ATOMIC_SEQ_CST)

	// AtomicMinFetch (Signed) — replaces the stored value with the smaller of it and the provided value, returning the result.

#ifndef AtomicMinFetchLong
#if CORIUM_COMPILER_MSVC
#define AtomicMinFetchLong(p_Ptr, v_Value, memOrder) \
        _InterlockedMin((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_min_fetch)
#define AtomicMinFetchLong(p_Ptr, v_Value, memOrder) \
        __atomic_min_fetch((p_Ptr), (v_Value), (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_min_fetch"
#endif
#else
#error "Unsupported compiler for atomic min fetch (signed)"
#endif
#endif

// 1. Success: RELAXED
#define AtomicMinFetchLong_Relaxed(p_Ptr, v_Value) \
        AtomicMinFetchLong((p_Ptr), (v_Value), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicMinFetchLong_Acquire(p_Ptr, v_Value) \
        AtomicMinFetchLong((p_Ptr), (v_Value), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicMinFetchLong_Release(p_Ptr, v_Value) \
        AtomicMinFetchLong((p_Ptr), (v_Value), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicMinFetchLong_AcqRel(p_Ptr, v_Value) \
        AtomicMinFetchLong((p_Ptr), (v_Value), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicMinFetchLong_SeqCst(p_Ptr, v_Value) \
        AtomicMinFetchLong((p_Ptr), (v_Value), __ATOMIC_SEQ_CST)

	// AtomicMaxFetch (Signed) — replaces the stored value with the larger of it and the provided value, returning the result.

#ifndef AtomicMaxFetchLong
#if CORIUM_COMPILER_MSVC
#define AtomicMaxFetchLong(p_Ptr, v_Value, memOrder) \
        _InterlockedMax((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_max_fetch)
#define AtomicMaxFetchLong(p_Ptr, v_Value, memOrder) \
        __atomic_max_fetch((p_Ptr), (v_Value), (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_max_fetch"
#endif
#else
#error "Unsupported compiler for atomic max fetch (signed)"
#endif
#endif

// 1. Success: RELAXED
#define AtomicMaxFetchLong_Relaxed(p_Ptr, v_Value) \
        AtomicMaxFetchLong((p_Ptr), (v_Value), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicMaxFetchLong_Acquire(p_Ptr, v_Value) \
        AtomicMaxFetchLong((p_Ptr), (v_Value), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicMaxFetchLong_Release(p_Ptr, v_Value) \
        AtomicMaxFetchLong((p_Ptr), (v_Value), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicMaxFetchLong_AcqRel(p_Ptr, v_Value) \
        AtomicMaxFetchLong((p_Ptr), (v_Value), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicMaxFetchLong_SeqCst(p_Ptr, v_Value) \
        AtomicMaxFetchLong((p_Ptr), (v_Value), __ATOMIC_SEQ_CST)

	// AtomicMinFetch (Unsigned) — replaces the stored value with the smaller of it and the provided value (unsigned compare), returning the result.

#ifndef AtomicMinFetchULong
#if CORIUM_COMPILER_MSVC
#define AtomicMinFetchULong(p_Ptr, v_Value, memOrder) \
        _InterlockedUMin((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_min_fetch)
#define AtomicMinFetchULong(p_Ptr, v_Value, memOrder) \
        __atomic_min_fetch((p_Ptr), (v_Value), (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_min_fetch"
#endif
#else
#error "Unsupported compiler for atomic min fetch (unsigned)"
#endif
#endif

// 1. Success: RELAXED
#define AtomicMinFetchULong_Relaxed(p_Ptr, v_Value) \
        AtomicMinFetchULong((p_Ptr), (v_Value), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicMinFetchULong_Acquire(p_Ptr, v_Value) \
        AtomicMinFetchULong((p_Ptr), (v_Value), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicMinFetchULong_Release(p_Ptr, v_Value) \
        AtomicMinFetchULong((p_Ptr), (v_Value), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicMinFetchULong_AcqRel(p_Ptr, v_Value) \
        AtomicMinFetchULong((p_Ptr), (v_Value), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicMinFetchULong_SeqCst(p_Ptr, v_Value) \
        AtomicMinFetchULong((p_Ptr), (v_Value), __ATOMIC_SEQ_CST)

	// AtomicMaxFetch (Unsigned) — replaces the stored value with the larger of it and the provided value (unsigned compare), returning the result.

#ifndef AtomicMaxFetchULong
#if CORIUM_COMPILER_MSVC
#define AtomicMaxFetchULong(p_Ptr, v_Value, memOrder) \
        _InterlockedUMax((p_Ptr), (v_Value))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_max_fetch)
#define AtomicMaxFetchULong(p_Ptr, v_Value, memOrder) \
        __atomic_max_fetch((p_Ptr), (v_Value), (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_max_fetch"
#endif
#else
#error "Unsupported compiler for atomic max fetch (unsigned)"
#endif
#endif

// 1. Success: RELAXED
#define AtomicMaxFetchULong_Relaxed(p_Ptr, v_Value) \
        AtomicMaxFetchULong((p_Ptr), (v_Value), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicMaxFetchULong_Acquire(p_Ptr, v_Value) \
        AtomicMaxFetchULong((p_Ptr), (v_Value), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicMaxFetchULong_Release(p_Ptr, v_Value) \
        AtomicMaxFetchULong((p_Ptr), (v_Value), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicMaxFetchULong_AcqRel(p_Ptr, v_Value) \
        AtomicMaxFetchULong((p_Ptr), (v_Value), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicMaxFetchULong_SeqCst(p_Ptr, v_Value) \
        AtomicMaxFetchULong((p_Ptr), (v_Value), __ATOMIC_SEQ_CST)

	// AtomicTestAndSet — atomically sets a single bit, returning its previous state.

#ifndef AtomicTestAndSet
#if CORIUM_COMPILER_MSVC
#define AtomicTestAndSet(p_Ptr, bit, memOrder) \
        _InterlockedBitTestAndSet((p_Ptr), (bit))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_test_and_set)
#define AtomicTestAndSet(p_Ptr, bit, memOrder) \
        __atomic_test_and_set((p_Ptr), (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_test_and_set"
#endif
#else
#error "Unsupported compiler for atomic test and set"
#endif
#endif

// 1. Success: RELAXED
#define AtomicTestAndSet_Relaxed(p_Ptr, bit) \
        AtomicTestAndSet((p_Ptr), (bit), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicTestAndSet_Acquire(p_Ptr, bit) \
        AtomicTestAndSet((p_Ptr), (bit), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicTestAndSet_Release(p_Ptr, bit) \
        AtomicTestAndSet((p_Ptr), (bit), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicTestAndSet_AcqRel(p_Ptr, bit) \
        AtomicTestAndSet((p_Ptr), (bit), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicTestAndSet_SeqCst(p_Ptr, bit) \
        AtomicTestAndSet((p_Ptr), (bit), __ATOMIC_SEQ_CST)

	// AtomicClear — atomically clears a single bit, returning its previous state.

#ifndef AtomicClear
#if CORIUM_COMPILER_MSVC
#define AtomicClear(p_Ptr, bit, memOrder) \
        _InterlockedBitTestAndReset((p_Ptr), (bit))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#if __has_builtin(__atomic_clear)
#define AtomicClear(p_Ptr, bit, memOrder) \
        __atomic_clear((p_Ptr), (memOrder))
#else
#error "Missing builtin GNU intrinsic: __atomic_clear"
#endif
#else
#error "Unsupported compiler for atomic clear"
#endif
#endif

// 1. Success: RELAXED
#define AtomicClear_Relaxed(p_Ptr, bit) \
        AtomicClear((p_Ptr), (bit), __ATOMIC_RELAXED)

// 2. Success: ACQUIRE
#define AtomicClear_Acquire(p_Ptr, bit) \
        AtomicClear((p_Ptr), (bit), __ATOMIC_ACQUIRE)

// 3. Success: RELEASE
#define AtomicClear_Release(p_Ptr, bit) \
        AtomicClear((p_Ptr), (bit), __ATOMIC_RELEASE)

// 4. Success: ACQ_REL
#define AtomicClear_AcqRel(p_Ptr, bit) \
        AtomicClear((p_Ptr), (bit), __ATOMIC_ACQ_REL)

// 5. Success: SEQ_CST
#define AtomicClear_SeqCst(p_Ptr, bit) \
        AtomicClear((p_Ptr), (bit), __ATOMIC_SEQ_CST)
}
