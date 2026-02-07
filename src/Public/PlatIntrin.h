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
	enum class CompilerFrontend : uint8_t
	{
		CompilerFrontend_Msvc = 1,
		CompilerFrontend_Clang = 2,
		CompilerFrontend_Gcc = 3
	};

#if defined(__clang__)
	[[maybe_unused]] constexpr CompilerFrontend kCompilerFrontend = CompilerFrontend::CompilerFrontend_Clang;
#elif defined(__GNUC__)
	[[maybe_unused]] constexpr CompilerFrontend kCompilerFrontend = CompilerFrontend::CompilerFrontend_Gcc;
#elif defined(_MSC_VER)
	[[maybe_unused]] constexpr CompilerFrontend kCompilerFrontend = CompilerFrontend::CompilerFrontend_Msvc;
#else
#error Unsupported compiler frontend
#endif

	ForceInline CORIUM constexpr [[maybe_unused]] CompilerFrontend frontend() {
		return kCompilerFrontend;
	}

	enum class CompilerBackend : uint8_t {
		CompilerBackend_Msvc = 1,
		CompilerBackend_Gcc = 2
	};

#if defined(__clang__) || defined(__GNUC__)
	[[maybe_unused]] constexpr CompilerBackend kCompilerBackend = Corium::Intrinsic::CompilerBackend::CompilerBackend_Gcc;
#elif defined(_MSC_VER)
	[[maybe_unused]] constexpr CompilerBackend kCompilerBackend = Corium::Intrinsic::CompilerBackend::CompilerBackend_Msvc;
#else
#error "Unsupported compiler backend"
#endif

	ForceInline CORIUM constexpr [[maybe_unused]] CompilerBackend backend() {
		return kCompilerBackend;
	}

	// Don't want some random compiler screaming cause some constant vanished in linux
#if !defined(__clang__) && !defined(__GNUC__)
#define __ATOMIC_RELAXED 0
#define __ATOMIC_RELAXED 0
#define __ATOMIC_RELEASE 0
#define __ATOMIC_ACQUIRE 0
#define __ATOMIC_ACQ_REL 0
#define __ATOMIC_SEQ_CST 0
#endif

	// Section containing hardware fences.

	// ============================================================================
	// FullFence
	//
	// Establishes a global hardware ordering point.
	//
	// Guarantees that all loads and stores issued before this point become
	// visible before any loads or stores issued after it.
	//
	// This is a heavyweight synchronization primitive intended only for
	// global phase transitions, device boundaries, or shutdown paths.
	// ============================================================================

	ForceInline CORIUM void FullFence() {
		if constexpr (backend() == CompilerBackend::CompilerBackend_Msvc) _mm_mfence();
		else if constexpr (backend() == CompilerBackend::CompilerBackend_Gcc) {
#if defined(__has_builtin)
#if __has_builtin(__atomic_thread_fence)
			__atomic_thread_fence(__ATOMIC_SEQ_CST);
#else
			static_assert(__has_builtin(__atomic_thread_fence), "__atomic_thread_fence not supported by this compiler");
#endif
#else
			static_assert(false, "__has_builtin not available");
#endif
		} else {
			static_assert(false, "Unsupported full fence in this environment");
			Unreachable();
		}
	}

	// ============================================================================
	// LoadFence
	//
	// Establishes ordering for load operations.
	//
	// Prevents later loads from being observed before earlier loads.
	// Does not impose ordering on stores.
	//
	// Intended for explicit consumption of published data.
	// ============================================================================

	ForceInline	CORIUM void LoadFence() {
		if constexpr (backend() == CompilerBackend::CompilerBackend_Msvc) _mm_lfence();
		else if constexpr (backend() == CompilerBackend::CompilerBackend_Gcc) {
#if defined(__has_builtin)
#if __has_builtin(__atomic_thread_fence)
			__atomic_thread_fence(__ATOMIC_ACQUIRE);
#else
			static_assert(__has_builtin(__atomic_thread_fence), "__atomic_thread_fence not supported by this compiler");
#endif
#else
			static_assert(false, "__has_builtin not available");
#endif
		} else {
			static_assert(false, "Unsupported load fence in this environment");
			Unreachable();
		}
	}

	// ============================================================================
	// StoreFence
	//
	// Establishes ordering for store operations.
	//
	// Ensures that all prior stores are committed before subsequent stores.
	// Does not impose ordering on loads.
	//
	// Commonly used when publishing data followed by a visibility flag.
	// ============================================================================

	ForceInline CORIUM void StoreFence() {
		if constexpr (backend() == CompilerBackend::CompilerBackend_Msvc) _mm_sfence();
		else if constexpr (backend() == CompilerBackend::CompilerBackend_Gcc) {
#if defined(__has_builtin)
#if __has_builtin(__atomic_thread_fence)
			__atomic_thread_fence(__ATOMIC_RELEASE);
#else
			static_assert(__has_builtin(__atomic_thread_fence), "__atomic_thread_fence not supported by this compiler");
#endif
#else
			static_assert(false, "__has_builtin not available");
#endif
		} else {
			static_assert(false, "Unsupported store fence in this environment");
			Unreachable();
		}
	}

	// (Why is this even necessary?) Compiler hint fences

	// ============================================================================
	// RWCompileBarrier
	//
	// Compiler-only barrier for both loads and stores.
	//
	// Prevents the compiler from reordering memory operations across
	// this point, without emitting any CPU instructions.
	//
	// Does NOT provide inter-thread synchronization.
	// ============================================================================

	ForceInline CORIUM void RWCompileBarrier() {
		if constexpr (backend() == CompilerBackend::CompilerBackend_Msvc) _ReadWriteBarrier();
		else if constexpr (backend() == CompilerBackend::CompilerBackend_Gcc) {
#if defined(__has_builtin)
#if __has_builtin(__atomic_signal_fence)
			__atomic_signal_fence(__ATOMIC_SEQ_CST);
# else
			static_assert(__has_builtin(__atomic_signal_fence), "__atomic_signal_fence not supported by this compiler");
#endif
#else
			static_assert(false, "__has_builtin not available");
#endif
		} else {
			static_assert(false, "Unsupported read write barrier in this environment");
			Unreachable();
		}
	}

	// ============================================================================
	// RCompileBarrier
	//
	// Compiler-only barrier for load operations.
	//
	// Prevents reordering of reads across this point while allowing
	// stores to move freely.
	//
	// Intended for rare, read-only ordering constraints.
	// ============================================================================

	ForceInline CORIUM void RCompileBarrier() {
		if constexpr (backend() == CompilerBackend::CompilerBackend_Msvc) _ReadBarrier();
		else if constexpr (backend() == CompilerBackend::CompilerBackend_Gcc) {
#if defined(__has_builtin)
#if __has_builtin(__atomic_signal_fence)
			__atomic_signal_fence(__ATOMIC_ACQUIRE);
#else
			static_assert(__has_builtin(__atomic_signal_fence), "__atomic_signal_fence not supported by this compiler");
#endif
#else
			static_assert(false, "__has_builtin not available");
#endif
		} else {
			static_assert(false, "Unsupported read barrier in this environment");
			Unreachable();
		}
	}

	// ============================================================================
	// WCompileBarrier
	//
	// Compiler-only barrier for store operations.
	//
	// Prevents reordering of writes across this point while allowing
	// loads to move freely.
	//
	// Commonly used before publishing shared state.
	// ============================================================================

	ForceInline CORIUM void WCompileBarrier() {
		if constexpr (backend() == CompilerBackend::CompilerBackend_Msvc) {
			_WriteBarrier();
		} else if constexpr (backend() == CompilerBackend::CompilerBackend_Gcc) {
#if defined(__has_builtin)
#if __has_builtin(__atomic_signal_fence)
			__atomic_signal_fence(__ATOMIC_RELEASE);
#else
			static_assert(false, "__atomic_signal_fence not supported by this compiler");
#endif
#else
			static_assert(false, "__has_builtin not available");
#endif
		} else {
			static_assert(false, "Unsupported write barrier in this environment");
		}
	}

	template<typename T>
	struct ValidAtomicParameter final {
		using Type = T;
		static_assert(
			std::is_trivially_copyable_v<Type>,
			"Unsupported atomic nature: type must be trivially copyable."
			);

		static_assert(
			!std::is_const_v<Type>,
			"Unsupported atomic nature: atomic type must not be const-qualified."
			);

		static_assert(
			!std::is_volatile_v<Type>,
			"Unsupported atomic nature: atomic type must not be volatile-qualified."
			);
	};

	// Moving on to a wall of macros, fuck!!

	// ============================================================================
	// AtomicLoad
	//
	// Atomically reads the value stored at the target address.
	//
	// Guarantees that the value is read as a single, indivisible operation
	// even in the presence of concurrent writers.
	//
	// Does not modify the stored value.
	// ============================================================================

#ifndef AtomicLoad_Relaxed
#define AtomicLoad_Relaxed
#if defined(_MSC_VER)
#define AtomicLoad_Relaxed(p_Ptr) \
    (*reinterpret_cast<volatile std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr))
#elif defined(__clang__) || defined(__GNUC__)
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
#define AtomicLoad_Acquire
#if defined(_MSC_VER)
#define AtomicLoad_Acquire(p_Ptr) \
    (*reinterpret_cast<volatile std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr))

#elif defined(__clang__) || defined(__GNUC__)

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
#define AtomicLoad_Consume
#if defined(_MSC_VER)
	/* CONSUME collapses to ACQUIRE */
#define AtomicLoad_Consume(p_Ptr) \
    (*reinterpret_cast<volatile std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr))
#elif defined(__clang__) || defined(__GNUC__)
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
#define AtomicLoad_SeqCst
#if defined(_MSC_VER)
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
#elif defined(__clang__) || defined(__GNUC__)
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

	// ============================================================================
	// AtomicStore
	//
	// Atomically writes a value to the target address.
	//
	// Guarantees that the write is visible as a single, indivisible update
	// to all threads observing the location.
	//
	// Overwrites the previous value.
	// ============================================================================


#ifndef AtomicStore_Relaxed
#define AtomicStore_Relaxed
#if defined(_MSC_VER)

#define AtomicStore_Relaxed(p_Ptr, v_Value)		  \
		(*reinterpret_cast<volatile std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr) = (v_Value))
#elif defined(__clang__) || defined(__GNUC__)
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
#define AtomicStore_Release
#if defined(_MSC_VER)

#define AtomicStore_Release(p_Ptr, v_Value)		  \
	(*reinterpret_cast<volatile std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr) = (v_Value))
#elif defined(__clang__) || defined(__GNUC__)
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
#define AtomicStore_SeqCst
#if defined(_MSC_VER)
/*
* MSVC x64 has no standalone seq_cst store.
* Volatile store + full fence is the strongest representable form.
*/
#define AtomicStore_SeqCst(p_Ptr, v_Value)		  \
	([&]() {                                                           \
		*reinterpret_cast<volatile std::remove_pointer_t<decltype(p_Ptr)>*>(p_Ptr) = v_Value;			 \
		_mm_mfence();                                                   \
		}())
#elif defined(__clang__) || defined(__GNUC__)
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

	// ============================================================================
	// AtomicExchange
	//
	// Atomically replaces the value at the target address with a new value.
	//
	// Returns the previous value stored at the address.
	//
	// Commonly used for state transitions, flags, and lock primitives.
	// ============================================================================

#ifndef AtomicExchange32_Relaxed
#define AtomicExchange32_Relaxed
#if defined(_MSC_VER)
#define AtomicExchange32_Relaxed(p_Ptr, v_Value) \
		_InterlockedExchange((p_Ptr, (v_Value)))
#elif defined(__clang__) || defined(__GNUC__)
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
#define AtomicExchange64_Relaxed
#if defined(_MSC_VER)
#define AtomicExchange64_Relaxed(p_Ptr, v_Value) \
		_InterlockedExchange64((p_Ptr, (v_Value)))
#elif defined(__clang__) || defined(__GNUC__)
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
#define AtomicExchange32_AcqRel
#if defined(_MSC_VER)
#define AtomicExchange32_AcqRel(p_Ptr, v_Value) \
		_InterlockedExchange((p_Ptr, (v_Value)))
#elif defined(__clang__) || defined(__GNUC__)
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
#define AtomicExchange64_AcqRel
#if defined(_MSC_VER)
#define AtomicExchange64_AcqRel(p_Ptr, v_Value) \
		_InterlockedExchange64((p_Ptr, (v_Value)))
#elif defined(__clang__) || defined(__GNUC__)
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
#define AtomicExchange32_SeqCst
#if defined(_MSC_VER)
#define AtomicExchange32_SeqCst(p_Ptr, v_Value) \
		_InterlockedExchange((p_Ptr, (v_Value)))
#elif defined(__clang__) || defined(__GNUC__)
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
#define AtomicExchange64_SeqCst
#if defined(_MSC_VER)
#define AtomicExchange64_SeqCst(p_Ptr, v_Value) \
		_InterlockedExchange64((p_Ptr, (v_Value)))
#elif defined(__clang__) || defined(__GNUC__)
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

	// ============================================================================
	// AtomicCompareExchange
	//
	// Atomically compares the current value at the target address with an
	// expected value, and conditionally replaces it with a desired value.
	//
	// Returns whether the replacement occurred, and updates the expected
	// value on failure.
	//
	// Fundamental primitive for lock-free algorithms.
	// ============================================================================


#ifndef AtomicCompareExchange32
#define AtomicCompareExchange32
#if defined(_MSC_VER)
#define AtomicCompareExchange32(p_Ptr, expected, desired, weak, success_memOrder, failure_memOrder) \
        _InterlockedCompareExchange((p_Ptr), (desired), *(expected))
#elif defined(__clang__) || defined(__GNUC__)
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
#define AtomicCompareExchange64
#if defined(_MSC_VER)
#define AtomicCompareExchange64(p_Ptr, expected, desired, weak, success_memOrder, failure_memOrder) \
        _InterlockedCompareExchange64((p_Ptr), (desired), *(expected))
#elif defined(__clang__) || defined(__GNUC__)
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

	// ============================================================================
	// AtomicFetchAdd
	//
	// Atomically adds a value to the target address.
	//
	// Returns the value that was stored prior to the addition.
	//
	// Commonly used for counters and reference tracking.
	// ============================================================================

#ifndef AtomicFetchAdd32
#define AtomicFetchAdd32
#if defined(_MSC_VER)
#define AtomicFetchAdd32(p_Ptr, v_Value, memOrder) \
        _InterlockedExchangeAdd((p_Ptr), (v_Value))
#elif defined(__clang__) || defined(__GNUC__)
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
#define AtomicFetchAdd64
#if defined(_MSC_VER)
#define AtomicFetchAdd64(p_Ptr, v_Value, memOrder) \
        _InterlockedExchangeAdd64((p_Ptr), (v_Value))
#elif defined(__clang__) || defined(__GNUC__)
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

	// ============================================================================
	// AtomicFetchAdd
	//
	// Atomically adds a value to the target address.
	//
	// Returns the value that was stored prior to the addition.
	//
	// Commonly used for counters and reference tracking.
	// ============================================================================

#ifndef AtomicAddFetch32
#define AtomicAddFetch32
#if defined(_MSC_VER)
#define AtomicAddFetch32(p_Ptr, v_Value) \
        _InterlockedExchangeAdd((p_Ptr), (v_Value)) + (v_Value)
#elif defined(__clang__) || defined(__GNUC__)
#if __has_builtin(__atomic_add_fetch)
#define AtomicAddFetch32(p_Ptr, v_Value) \
        __atomic_add_fetch((p_Ptr), (v_Value), __ATOMIC_RELAXED)
#else
#error "Missing builtin GNU intrinsic: __atomic_add_fetch"
#endif
#else
#error "Unsupported compiler for atomic add fetch"
#endif
#endif

// 1. Success: RELAXED
#define AtomicAddFetch32_Relaxed(p_Ptr, v_Value) \
        AtomicAddFetch32((p_Ptr), (v_Value))

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
#define AtomicAddFetch64
#if defined(_MSC_VER)
#define AtomicAddFetch64(p_Ptr, v_Value) \
        _InterlockedExchangeAdd64((p_Ptr), (v_Value)) + (v_Value)
#elif defined(__clang__) || defined(__GNUC__)
#if __has_builtin(__atomic_add_fetch)
#define AtomicAddFetch64(p_Ptr, v_Value) \
        __atomic_add_fetch((p_Ptr), (v_Value), __ATOMIC_RELAXED)
#else
#error "Missing builtin GNU intrinsic: __atomic_add_fetch"
#endif
#else
#error "Unsupported compiler for atomic add fetch"
#endif
#endif

// 1. Success: RELAXED
#define AtomicAddFetch64_Relaxed(p_Ptr, v_Value) \
        AtomicAddFetch64((p_Ptr), (v_Value))

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

	// ============================================================================
	// AtomicIncrement
	//
	// Atomically increments the value at the target address by one.
	//
	// Equivalent to an atomic add of +1.
	//
	// Frequently used for reference counts and event counters.
	// ============================================================================

#ifndef AtomicIncrement32
#define AtomicIncrement32
#if defined(_MSC_VER)
#define AtomicIncrement32(p_Ptr) \
        _InterlockedIncrement((p_Ptr))
#elif defined(__clang__) || defined(__GNUC__)
#if __has_builtin(__atomic_fetch_add)
#define AtomicIncrement32(p_Ptr) \
        __atomic_add_fetch((p_Ptr), 1, __ATOMIC_RELAXED)
#else
#error "Missing builtin GNU intrinsic: __atomic_add_fetch"
#endif
#else
#error "Unsupported compiler for atomic increment"
#endif
#endif

// 1. Success: RELAXED
#define AtomicIncrement32_Relaxed(p_Ptr) \
        AtomicIncrement32((p_Ptr))

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
#define AtomicDecrement32
#if defined(_MSC_VER)
#define AtomicDecrement32(p_Ptr) \
        _InterlockedDecrement((p_Ptr))
#elif defined(__clang__) || defined(__GNUC__)
#if __has_builtin(__atomic_fetch_sub)
#define AtomicDecrement32(p_Ptr) \
        __atomic_sub_fetch((p_Ptr), 1, __ATOMIC_RELAXED)
#else
#error "Missing builtin GNU intrinsic: __atomic_sub_fetch"
#endif
#else
#error "Unsupported compiler for atomic decrement"
#endif
#endif

// 1. Success: RELAXED
#define AtomicDecrement32_Relaxed(p_Ptr) \
        AtomicDecrement32((p_Ptr))

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
#define AtomicIncrement64
#if defined(_MSC_VER)
#define AtomicIncrement64(p_Ptr) \
        _InterlockedIncrement64((p_Ptr))
#elif defined(__clang__) || defined(__GNUC__)
#if __has_builtin(__atomic_fetch_add)
#define AtomicIncrement64(p_Ptr) \
        __atomic_add_fetch((p_Ptr), 1, __ATOMIC_RELAXED)
#else
#error "Missing builtin GNU intrinsic: __atomic_add_fetch"
#endif
#else
#error "Unsupported compiler for atomic increment"
#endif
#endif

// 1. Success: RELAXED
#define AtomicIncrement64_Relaxed(p_Ptr) \
        AtomicIncrement64((p_Ptr))

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

	// ============================================================================
	// AtomicDecrement
	//
	// Atomically decrements the value at the target address by one.
	//
	// Equivalent to an atomic add of -1.
	//
	// Often paired with lifetime or ownership tracking.
	// ============================================================================

#ifndef AtomicDecrement64
#define AtomicDecrement64
#if defined(_MSC_VER)
#define AtomicDecrement64(p_Ptr) \
        _InterlockedDecrement64((p_Ptr))
#elif defined(__clang__) || defined(__GNUC__)
#if __has_builtin(__atomic_fetch_sub)
#define AtomicDecrement64(p_Ptr) \
        __atomic_sub_fetch((p_Ptr), 1, __ATOMIC_RELAXED)
#else
#error "Missing builtin GNU intrinsic: __atomic_sub_fetch"
#endif
#else
#error "Unsupported compiler for atomic decrement"
#endif
#endif

// 1. Success: RELAXED
#define AtomicDecrement64_Relaxed(p_Ptr) \
        AtomicDecrement64((p_Ptr))

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

	// ============================================================================
	// AtomicFetchAnd
	//
	// Atomically applies a bitwise AND between the stored value and the
	// provided mask.
	//
	// Returns the previous value.
	//
	// Commonly used for flag clearing and masked state updates.
	// ============================================================================

#ifndef AtomicFetchAnd
#define AtomicFetchAnd
#if defined(_MSC_VER)
#define AtomicFetchAnd(p_Ptr, v_Value) \
        _InterlockedAnd((p_Ptr), (v_Value))
#elif defined(__clang__) || defined(__GNUC__)
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

	// ============================================================================
	// AtomicFetchOr
	//
	// Atomically applies a bitwise OR between the stored value and the
	// provided mask.
	//
	// Returns the previous value.
	//
	// Commonly used for flag setting and capability masks.
	// ============================================================================

#ifndef AtomicFetchOr
#define AtomicFetchOr
#if defined(_MSC_VER)
#define AtomicFetchOr(p_Ptr, v_Value) \
        _InterlockedOr((p_Ptr), (v_Value))
#elif defined(__clang__) || defined(__GNUC__)
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

	// ============================================================================
	// AtomicFetchXor
	//
	// Atomically applies a bitwise XOR between the stored value and the
	// provided mask.
	//
	// Returns the previous value.
	//
	// Useful for toggling bits or parity-style state changes.
	// ============================================================================

#ifndef AtomicFetchXor
#define AtomicFetchXor
#if defined(_MSC_VER)
#define AtomicFetchXor(p_Ptr, v_Value) \
        _InterlockedXor((p_Ptr), (v_Value))
#elif defined(__clang__) || defined(__GNUC__)
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

	// ============================================================================
	// AtomicFetchNand
	//
	// Atomically applies a bitwise NAND between the stored value and the
	// provided mask.
	//
	// Returns the previous value.
	//
	// Rarely used directly, but provided for completeness and symmetry.
	// ============================================================================

#ifndef AtomicFetchNand
#define AtomicFetchNand
#if defined(_MSC_VER)
#define AtomicFetchNand(p_Ptr, v_Value) \
        _InterlockedNand((p_Ptr), (v_Value))
#elif defined(__clang__) || defined(__GNUC__)
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

	// ============================================================================
	// AtomicMinFetch (Signed)
	//
	// Atomically replaces the stored value with the smaller of the current
	// value and the provided value.
	//
	// Returns the resulting value after the operation.
	//
	// Intended for monotonic lower-bound tracking.
	// ============================================================================

#ifndef AtomicMinFetchLong
#define AtomicMinFetchLong
#if defined(_MSC_VER)
#define AtomicMinFetchLong(p_Ptr, v_Value) \
        _InterlockedMin((p_Ptr), (v_Value))
#elif defined(__clang__) || defined(__GNUC__)
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

	// ============================================================================
	// AtomicMaxFetch (Signed)
	//
	// Atomically replaces the stored value with the larger of the current
	// value and the provided value.
	//
	// Returns the resulting value after the operation.
	//
	// Intended for monotonic upper-bound tracking.
	// ============================================================================

#ifndef AtomicMaxFetchLong
#define AtomicMaxFetchLong
#if defined(_MSC_VER)
#define AtomicMaxFetchLong(p_Ptr, v_Value) \
        _InterlockedMax((p_Ptr), (v_Value))
#elif defined(__clang__) || defined(__GNUC__)
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

	// ============================================================================
	// AtomicMinFetch (Unsigned)
	//
	// Atomically replaces the stored value with the smaller of the current
	// value and the provided value, using unsigned comparison.
	//
	// Returns the resulting value after the operation.
	// ============================================================================

#ifndef AtomicMinFetchULong
#define AtomicMinFetchULong
#if defined(_MSC_VER)
#define AtomicMinFetchULong(p_Ptr, v_Value) \
        _InterlockedUMin((p_Ptr), (v_Value))
#elif defined(__clang__) || defined(__GNUC__)
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

	// ============================================================================
	// AtomicMaxFetch (Unsigned)
	//
	// Atomically replaces the stored value with the larger of the current
	// value and the provided value, using unsigned comparison.
	//
	// Returns the resulting value after the operation.
	// ============================================================================

#ifndef AtomicMaxFetchULong
#define AtomicMaxFetchULong
#if defined(_MSC_VER)
#define AtomicMaxFetchULong(p_Ptr, v_Value) \
        _InterlockedUMax((p_Ptr), (v_Value))
#elif defined(__clang__) || defined(__GNUC__)
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

	// ============================================================================
	// AtomicTestAndSet
	//
	// Atomically sets a single bit and returns its previous state.
	//
	// Commonly used to implement simple spinlocks, flags, or once-only
	// execution guards.
	// ============================================================================

#ifndef AtomicTestAndSet
#define AtomicTestAndSet
#if defined(_MSC_VER)
#define AtomicTestAndSet(p_Ptr, bit) \
        _InterlockedBitTestAndSet((p_Ptr), (bit))
#elif defined(__clang__) || defined(__GNUC__)
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

	// ============================================================================
	// AtomicClear
	//
	// Atomically clears a single bit and returns its previous state.
	//
	// Typically paired with AtomicTestAndSet to manage bit-level flags.
	// ============================================================================

#ifndef AtomicClear
#define AtomicClear
#if defined(_MSC_VER)
#define AtomicClear(p_Ptr, bit) \
        _InterlockedBitTestAndReset((p_Ptr), (bit))
#elif defined(__clang__) || defined(__GNUC__)
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
