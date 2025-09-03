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
#include "Corium.h"

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

namespace Corium::Platform {
	constexpr size_t INVALID_HANDLE = 0xFFFFFFFFFFFFFFFF;

	inline size_t generateApiThreadID() noexcept {
		static std::atomic<uint64_t> counter{ 1 };

		uint64_t count = counter.fetch_add(1, std::memory_order_relaxed);
		uint64_t tsc = __rdtsc();
		uint64_t tid = reinterpret_cast<uint64_t>(&counter);

		return (count ^ tsc ^ tid) & 0xFFFFFFFFFFF;
	}

	using AffinityMask = unsigned long long;
	using ProcessorIdx = DWORD;

	// [Size in bytes]: 48
	alignas(64) struct CORIUM NativeThreadAttributes {
		AffinityMask m_Mask;
		bool m_IsDetached;
		int m_ThreadPriority;
		bool m_IsGuardPageEnabled;

		void canDetach(bool v_Detachable) {
			m_IsDetached = v_Detachable;
		}

		void setLaunchPriority(int v_Priority) {
			m_ThreadPriority = v_Priority;
		}

		void enableTlsGuard(bool v_Active) {
			m_IsGuardPageEnabled = v_Active;
		}

		template<size_t N>
		void coreMask(std::array<size_t, N>&& u_Affinities [[maybe_unused]] ) {
			for (size_t core : u_Affinities) m_Mask |= (1ULL << core);
		}

#if defined(_WIN32)
		GROUP_AFFINITY m_Affinity;
		DWORD m_IdealProcessor;
		bool m_SupportsIdealProcessor;
		bool m_SupportsGroup;

		template<size_t N>
		void groupAffinity(std::array<ProcessorIdx, N>&& u_Affinities [[maybe_unused]] ) {
			ProcessorIdx mask = 0ULL;
			for (ProcessorIdx core : u_Affinities) mask |= (1ULL << core);
			m_Mask = mask;
			m_Affinity.Mask = mask;
			m_SupportsGroup = true;
		}

		void setIdealProcessor(ProcessorIdx v_ProcIdx) {
			m_IdealProcessor = v_ProcIdx;
			m_SupportsIdealProcessor = true;
		}
#elif defined(__linux__)

#endif

		NativeThreadAttributes() {
			m_Mask = 0ULL;
			m_IsDetached = false;
			m_ThreadPriority = 0;
			m_IsGuardPageEnabled = false;
#if defined(_WIN32)
			m_Affinity = {};
			m_IdealProcessor = static_cast<ProcessorIdx>(-1);
			m_SupportsGroup = false;
			m_SupportsIdealProcessor = false;
#elif defined(__linux__)

#endif
		}
	};

	//[Size in Bytes]: 88
	alignas(128) struct CORIUM NativeThreadOptions {
		std::function<void()> m_StartPoint;
		const char* m_ThreadName;

		void launchFunction(auto&& u_LaunchFunction) {
			m_StartPoint = u_LaunchFunction;
			m_CanLaunch = true;
		}

#if defined(_WIN32)
		SIZE_T m_StackSize;
		bool m_IsPreSuspended;

		void reservedStackSize(SIZE_T v_StackSize) {
			m_StackSize = v_StackSize;
		}

		void StartSuspended(bool v_Suspended) {
			m_IsPreSuspended = v_Suspended;
		}
#elif defined(__linux_)
		size_t m_StackSize;
#endif

	private:
		bool m_CanLaunch;

	public:
		NativeThreadOptions(const char* pa_ThreadName) {
			m_StartPoint = nullptr;
			m_CanLaunch = false;
			m_ThreadName = pa_ThreadName;

#if defined(_WIN32)
			m_StackSize = 0;
			m_IsPreSuspended = false;
#elif defined(__linux__)
			m_StackSize = 0ULL;
#endif
		}
	};

	// [Size in bytes]: 48
	alignas(64) struct CORIUM CPUThreadHandle final{
	private:
		size_t m_CoriumThreadID;
		const char* m_ThreadName;
		bool m_IsRunning;
		bool m_IsClosed;
		bool m_IsValid;
#if defined(_WIN32)
		HANDLE m_ThreadHandle;
		DWORD m_CurrentThreadPriority;
		DWORD m_ExitCode;
		DWORD m_ThreadID;
#elif defined(__linux__)
		pthread_t m_ThreadObject;
		int m_ExitCode;
		pid_t m_ThreadID;
#endif

	public:
		CPUThreadHandle(const CPUThreadHandle&) = delete;
		CPUThreadHandle& operator=(const CPUThreadHandle&) = delete;

		[[nodiscard]] size_t getCoriumID() const noexcept {
			return m_CoriumThreadID;
		}

		[[nodiscard]] const char* getThreadName() const noexcept {
			return m_ThreadName;
		}

		[[nodiscard]] bool isRunning() const noexcept {
			return m_IsRunning;
		}

		[[nodiscard]] size_t getThreadPriority() const noexcept {
			return m_CurrentThreadPriority;
		}

		[[nodiscard]] size_t getExitCode() const noexcept {
			return m_ExitCode;
		}

		[[nodiscard]] size_t getSystemID() const noexcept {
			return m_ThreadID;
		}

		[[nodiscard]] bool isHandleClosed() const noexcept {
			return m_IsClosed;
		}

		CPUThreadHandle() = default;

	private:
		friend class NativeThread;
	};

	// [Size in bytes]: 8
	class ThreadHandle {
		size_t m_HandleID;
		explicit ThreadHandle(const size_t v_ID) : m_HandleID(v_ID) {}

	public:
		explicit ThreadHandle() : ThreadHandle(0) {}

		[[nodiscard]] bool isValid() const noexcept {
			return m_HandleID != INVALID_HANDLE;
		}
		friend class NativeThread;
	};

	struct TrampolineContext {
		std::function<void()> m_Func;
		std::function<void(std::function<void()> v_Callable)> m_TlsInit;
	};


	enum class Priority : int8_t {
		IDLE = THREAD_PRIORITY_IDLE,
		LOWEST = THREAD_PRIORITY_LOWEST,
		BELOW_NORMAL = THREAD_PRIORITY_BELOW_NORMAL,
		NORMAL = THREAD_PRIORITY_NORMAL,
		ABOVE_NORMAL = THREAD_PRIORITY_ABOVE_NORMAL,
		HIGHEST = THREAD_PRIORITY_HIGHEST,
		TIME_CRITICAL = THREAD_PRIORITY_TIME_CRITICAL
	};

	namespace this_platform_thread {
		static thread_local ThreadHandle t_Handle;
	}

	class NativeThread final {

		static std::vector<CPUThreadHandle> m_Handles;

		static DWORD WINAPI launch(PVOID p_Params) {
			const auto context = reinterpret_cast<TrampolineContext*>(p_Params);
			context->m_TlsInit(context->m_Func);

			delete context;
			return 0;
		}

		static void* pLaunch(void* po_Context) {
			const auto context = reinterpret_cast<TrampolineContext*>(po_Context);
			context->m_TlsInit(context->m_Func);

			delete context;
			return nullptr;
		}

		static void nullHandle(CPUThreadHandle& handle) {
			handle.m_ThreadName = nullptr;
			handle.m_CoriumThreadID = -1;
			handle.m_IsClosed = true;
			handle.m_CurrentThreadPriority = 0;
			handle.m_ExitCode = 1000;
			handle.m_IsRunning = false;
			handle.m_IsValid = false;
			handle.m_ThreadHandle = nullptr;
			handle.m_ThreadID = 0;
		}

	public:
		static ThreadHandle createThread(NativeThreadAttributes& ro_Attr, NativeThreadOptions& ro_Options);

		static bool detachThread(ThreadHandle& ro_Handle);

		static bool setPriority(const ThreadHandle& ro_Handle, Priority v_NewPriority);

		static bool closeHandle(CPUThreadHandle& ro_Handle);

		static CPUThreadHandle&& duplicate(const ThreadHandle& ro_Handle);

		static ProcessorIdx getCurrentProcessorNumber();

		static size_t getHardwareConcurrency();	

		static size_t suspendThread(const ThreadHandle& ro_Handle);

		static size_t resumeThread(const ThreadHandle& ro_Handle);

		static size_t joinThread(const ThreadHandle& ro_Handle);
	};
}