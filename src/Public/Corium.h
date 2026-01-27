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
#ifndef CORIUM
#if defined(_WIN32) || defined(__CYGWIN__)
#if defined(CORIUM_SHARED)
#define CORIUM __declspec(dllexport)
#else
#define CORIUM __declspec(dllimport)
#endif

#elif defined(__GNUC__) || defined(__clang__ )
#define CORIUM __attribute__((visibility("default")))
#else
#define CORIUM
#endif
#endif

#include <type_traits>
#include <functional>
#include <utility>
#include <concepts>
#include <array>
#include <optional>
#include <chrono>
#include <atomic>
#include <cstdint>

#if defined(_MSC_VER)
    #define ForceInline __forceinline
#elif defined(__GNUC__) || defined(__clang__)
    #define ForceInline inline __attribute__((always_inline))
#else
    #define ForceInline inline
#endif

#if defined(_MSC_VER)
#define Unreachable() __assume(0)
#elif defined(__GNUC__) || defined(__clang__)
#define Unreachable() __builtin_unreachable()
#else
#include <cstdlib>
#define Unreachable() std::abort()
#endif
