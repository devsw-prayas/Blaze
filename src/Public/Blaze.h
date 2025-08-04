#pragma once
#ifndef BLAZE
#define BLAZE __declspec(dllexport)
#endif
void BLAZE Init();

#include <type_traits>
#include <functional>
#include <utility>
#include <concepts>
#include <array>
#include <optional>
#include <chrono>

#if defined(_WIN32)
#include <Windows.h>
#elif defined(__linux__)
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <sched.h>
#elif defined(__APPLE__)
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <mach/mach.h>
#include <mach/thread_policy.h>
#endif



