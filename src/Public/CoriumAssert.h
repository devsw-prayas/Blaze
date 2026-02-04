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
#include <Corium.h>

#if defined(_MSC_VER)
#define CORIUM_DEBUG_BREAK() __debugbreak()
#elif defined(__GNUC__) || defined(__clang__)
#define CORIUM_DEBUG_BREAK() __builtin_trap()
#else
#include <cstdlib>
#define CORIUM_DEBUG_BREAK() std::abort()                                                       
#endif

#if !defined(CORIUM_DEBUG)
#if defined(_DEBUG) || defined(DEBUG)
#define CORIUM_DEBUG 1
#else
#define CORIUM_DEBUG 0
#endif
#endif

#if CORIUM_DEBUG

#define CORIUM_ASSERT(expr)                                      \
        do {                                                         \
            if (!(expr)) {                                          \
                CORIUM_DEBUG_BREAK();                               \
                Unreachable();                                      \
            }                                                        \
        } while (0)

#else

#define CORIUM_ASSERT(expr) Unreachable()
#endif

#if defined(CORIUM_DEBUG)
#define CORIUM_ASSUME(expr) CORIUM_ASSERT(expr)
#else
#define CORIUM_ASSUME(expr) __assume(expr)
#endif