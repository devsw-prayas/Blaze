#pragma once

#ifdef ALLOW_SYSCALL
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#elif defined(__linux__)
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#endif
#endif