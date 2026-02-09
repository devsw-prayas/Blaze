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

namespace Corium {
#if defined(_MSC_VER)
#define CORIUM_COMPILER_MSVC 1
#else
#define CORIUM_COMPILER_MSVC 0
#endif

#if defined(__clang__)
#define CORIUM_COMPILER_CLANG 1
#else
#define CORIUM_COMPILER_CLANG 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
#define CORIUM_COMPILER_GCC 1
#else
#define CORIUM_COMPILER_GCC 0
#endif
}

#if CORIUM_COMPILER_MSVC
#define CORIUM_FORCEINLINE __forceinline
#define CORIUM_NOINLINE    __declspec(noinline)
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#define CORIUM_FORCEINLINE inline __attribute__((always_inline))
#define CORIUM_NOINLINE    __attribute__((noinline))
#else
#define CORIUM_FORCEINLINE inline
#define CORIUM_NOINLINE
#endif

#define CORIUM_INLINE inline

#if CORIUM_COMPILER_MSVC
#define CORIUM_COMPILER_BARRIER() _ReadWriteBarrier()
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#define CORIUM_COMPILER_BARRIER() asm volatile("" ::: "memory")
#else
#define CORIUM_COMPILER_BARRIER()
#endif

#if CORIUM_COMPILER_MSVC
#define CORIUM_OPTIMIZE_OFF __pragma(optimize("", off))
#define CORIUM_OPTIMIZE_ON  __pragma(optimize("", on))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#define CORIUM_OPTIMIZE_OFF _Pragma("clang optimize off")
#define CORIUM_OPTIMIZE_ON  _Pragma("clang optimize on")
#else
#define CORIUM_OPTIMIZE_OFF
#define CORIUM_OPTIMIZE_ON
#endif

#if CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#define CORIUM_LIKELY(x)   __builtin_expect(!!(x), 1)
#define CORIUM_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define CORIUM_LIKELY(x)   (x)
#define CORIUM_UNLIKELY(x) (x)
#endif

#if CORIUM_COMPILER_MSVC
#define CORIUM_DEBUG_BREAK() __debugbreak()
#define CORIUM_TRAP()        __debugbreak()
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#define CORIUM_DEBUG_BREAK() __builtin_trap()
#define CORIUM_TRAP()        __builtin_trap()
#else
#include <cstdlib>
#define CORIUM_DEBUG_BREAK() std::abort()
#define CORIUM_TRAP()        std::abort()
#endif

#if CORIUM_COMPILER_MSVC
#define CORIUM_UNREACHABLE() __assume(0)
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#define CORIUM_UNREACHABLE() __builtin_unreachable()
#else
#define CORIUM_UNREACHABLE() CORIUM_TRAP()
#endif

#if CORIUM_COMPILER_MSVC
#define CORIUM_PRAGMA(x) __pragma(x)
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#define CORIUM_PRAGMA(x) _Pragma(#x)
#else
#define CORIUM_PRAGMA(x)
#endif

#define CORIUM_DIAGNOSTIC_PUSH CORIUM_PRAGMA(diagnostic push)
#define CORIUM_DIAGNOSTIC_POP  CORIUM_PRAGMA(diagnostic pop)

#if CORIUM_COMPILER_MSVC
#define CORIUM_DISABLE_WARNING(w) CORIUM_PRAGMA(warning(disable : w))
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#define CORIUM_DISABLE_WARNING(w) CORIUM_PRAGMA(clang diagnostic ignored w)
#else
#define CORIUM_DISABLE_WARNING(w)
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(fallthrough)
#define CORIUM_FALLTHROUGH [[fallthrough]]
#else
#define CORIUM_FALLTHROUGH
#endif
#else
#define CORIUM_FALLTHROUGH
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(nodiscard)
#define CORIUM_NODISCARD [[nodiscard]]
#if __cplusplus >= 202002L
#define CORIUM_NODISCARD_MSG(msg) [[nodiscard(msg)]]
#else
#define CORIUM_NODISCARD_MSG(msg) [[nodiscard]]
#endif
#else
#define CORIUM_NODISCARD
#define CORIUM_NODISCARD_MSG(msg)
#endif
#else
#define CORIUM_NODISCARD
#define CORIUM_NODISCARD_MSG(msg)
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(maybe_unused)
#define CORIUM_MAYBE_UNUSED [[maybe_unused]]
#else
#define CORIUM_MAYBE_UNUSED
#endif
#else
#define CORIUM_MAYBE_UNUSED
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(deprecated)
#define CORIUM_DEPRECATED [[deprecated]]
#define CORIUM_DEPRECATED_MSG(msg) [[deprecated(msg)]]
#else
#define CORIUM_DEPRECATED
#define CORIUM_DEPRECATED_MSG(msg)
#endif
#else
#define CORIUM_DEPRECATED
#define CORIUM_DEPRECATED_MSG(msg)
#endif

#if defined(__has_cpp_attribute)
#if __has_cpp_attribute(noreturn)
#define CORIUM_NORETURN [[noreturn]]
#else
#define CORIUM_NORETURN
#endif
#else
#define CORIUM_NORETURN
#endif

#if CORIUM_COMPILER_MSVC
#define CORIUM_RESTRICT __restrict
#elif CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#define CORIUM_RESTRICT __restrict__
#else
#define CORIUM_RESTRICT
#endif

#define CORIUM_ALIGNAS(n) alignas(n)

#if CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#define CORIUM_ASSUME_ALIGNED(ptr, n) __builtin_assume_aligned((ptr), (n))
#else
#define CORIUM_ASSUME_ALIGNED(ptr, n) (ptr)
#endif

#if CORIUM_COMPILER_CLANG || CORIUM_COMPILER_GCC
#define CORIUM_HOT  __attribute__((hot))
#define CORIUM_COLD __attribute__((cold))
#else
#define CORIUM_HOT
#define CORIUM_COLD
#endif
