#include "Corium.h"
#include "ThreadPlatform.h"

namespace Corium::Platform {
	std::vector<CPUThreadHandle> NativeThread::m_Handles{};

	ThreadHandle NativeThread::createThread(NativeThreadAttributes& ro_Attr, NativeThreadOptions& ro_Options) {
		//Setting up internal thread object and opaque handle
		CPUThreadHandle threadHandle;
		threadHandle.m_ThreadName = ro_Options.m_ThreadName;
		threadHandle.m_CoriumThreadID = generateApiThreadID();
		ThreadHandle opaqueHandle{ threadHandle.m_CoriumThreadID };
		threadHandle.m_IsClosed = false;

#if defined(_WIN32)
		DWORD lpThreadId;

		//Setting up thread attributes
		LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList = nullptr;
		SIZE_T size = 0;
		InitializeProcThreadAttributeList(nullptr, 2, 0, &size);
		lpAttributeList = static_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(HeapAlloc(GetProcessHeap(), 0, size));
		if (!lpAttributeList) return ThreadHandle(INVALID_HANDLE);
		InitializeProcThreadAttributeList(lpAttributeList, 2, 0, &size);

		const auto l_Cleanup = [&]() {
			DeleteProcThreadAttributeList(lpAttributeList);
			HeapFree(GetProcessHeap(), 0, lpAttributeList);
			};
		//If group affinity is supported
		if (ro_Attr.m_SupportsGroup)
			if (!UpdateProcThreadAttribute(lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_GROUP_AFFINITY, &ro_Attr.m_Affinity,
				sizeof(GROUP_AFFINITY), nullptr, nullptr)) {
				l_Cleanup();
				return ThreadHandle(INVALID_HANDLE);
			}

		//If Ideal processor is hinted
		if (ro_Attr.m_SupportsIdealProcessor)
			if (!UpdateProcThreadAttribute(lpAttributeList, 0, PROC_THREAD_ATTRIBUTE_IDEAL_PROCESSOR, &ro_Attr.m_IdealProcessor,
				sizeof(ProcessorIdx), nullptr, nullptr)) {
				l_Cleanup();
				return ThreadHandle(INVALID_HANDLE);
			}

		//TLS pre launch init
		auto l_TlsInit = [opaqueHandle, &threadHandle](const std::function<void()>& v_callable) {
			this_platform_thread::t_Handle = opaqueHandle;
			this_platform_thread::t_ParkingPermit.m_Id = threadHandle.m_CoriumThreadID;
#if defined(__linux__)
			pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
			pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, nullptr);
#endif
			v_callable();
			};

		//Launch trampoline context
		auto context = new TrampolineContext;
		context->m_TlsInit = std::move(l_TlsInit);
		context->m_Func = std::move(ro_Options.m_StartPoint);

		//Creating the thread
		HANDLE kernelHandle = CreateRemoteThreadEx(GetCurrentProcess(), nullptr, ro_Options.m_StackSize,
			launch, context, CREATE_SUSPENDED, lpAttributeList, &lpThreadId);
		if (!kernelHandle) {
			delete context;
			return ThreadHandle(INVALID_HANDLE);
		}

		threadHandle.m_ThreadID = lpThreadId;
		threadHandle.m_ThreadHandle = kernelHandle;

		//Naming the thread
		int length = MultiByteToWideChar(CP_UTF8, 0, ro_Options.m_ThreadName,
			-1, nullptr, 0);
		if (length > 0) {
			std::wstring wThreadName(length, L'\0');
			MultiByteToWideChar(CP_UTF8, 0, ro_Options.m_ThreadName,
				-1, wThreadName.data(), length);
			SetThreadDescription(kernelHandle, wThreadName.c_str());
		}

		//Post launch init
		SetThreadAffinityMask(kernelHandle, ro_Attr.m_Mask);
		SetThreadPriority(kernelHandle, ro_Attr.m_ThreadPriority);
		threadHandle.m_CurrentThreadPriority = ro_Attr.m_ThreadPriority;

		if (ro_Attr.m_IsDetached) CloseHandle(kernelHandle);
		else m_Handles.push_back(std::move(threadHandle));

		//Cleanup
		l_Cleanup();

#elif defined(__linux__)
		pthread_t thread;

		//Setting up thread attributes
		pthread_attr_t attr;
		pthread_attr_init(&attr);

		if (pthread_attr_setstacksize(&attr, ro_Options.m_StackSize)) return ThreadHandle(INVALID_HANDLE);
		if (ro_Attr.m_IsDetached) {
			if (pthread_attr_setdetachedstate(&attr, PTHREAD_CREATE_DETACHED)) return ThreadHandle(INVALID_HANDLE);
		}

		//Pre-launch TLS setup
		auto l_TlsInit = [opaqueHandle](const std::function<void()>& v_callable) {
			this_platform_thread::t_Handle = opaqueHandle;
			v_callable();
			};

		//Launch trampoline context init
		auto context = new TrampolineContext;
		context->m_TlsInit = std::move(l_TlsInit);
		context->m_Func = std::move(ro_Options.m_StartPoint);

		//Launching thread
		int state = pthread_create(&thread, &attr, pLaunch, context);
		if (state) {
			delete context;
			return ThreadHandle(INVALID_HANDLE);
		}

		//Naming the thread
		pthread_setname_np(thread, ro_Options.m_ThreadName);

		//Post launch init
		threadHandle.m_ThreadObject = thread;
		threadHandle.m_ThreadID = reinterpret_cast<uint64_t>(thread);

		//CPU affinity
		cpu_set_t affinityMask;
		CPU_ZERO(&affinityMask);

		size_t bitmask = ro_Attr.m_Mask;
		for (int core = 0; core < 64; core++) {
			if (bitmask & (1ULL << core)) {
				CPU_SET(core, &affinityMask);
			}
		}

		//Cleanup
		pthread_attr_destroy(&attr);
#endif

		if (!ro_Options.m_IsPreSuspended) {
			resumeThread(opaqueHandle);
		}
		return opaqueHandle;
	}

	bool NativeThread::detachThread(ThreadHandle& ro_Handle) {
		if (!ro_Handle.isValid()) return false;
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [&ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
			return ro_Handle.m_HandleID == ro_InternalHandle.getCoriumID();
			});

		if (itr != m_Handles.end()) {
#if defined(_WIN32)
			CloseHandle(itr->m_ThreadHandle);
			itr = m_Handles.erase(itr);
			ro_Handle.m_HandleID = INVALID_HANDLE;
			return true;
#elif defined(__linux__)
			pthread_detach(itr->m_ThreadObject);
			itr = m_Handles.erase(itr);
			ro_Handle.m_HandleID = INVALID_HANDLE;
			return true;
#endif
		}
		return false;
	}

	bool NativeThread::setPriority(const ThreadHandle& ro_Handle, Priority v_NewPriority) {
		if (!ro_Handle.isValid()) return false;
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [&ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
			return ro_Handle.m_HandleID == ro_InternalHandle.getCoriumID();
			});
		if (itr == m_Handles.end()) return false;

#if defined(_WIN32)
		return SetThreadPriority(itr->m_ThreadHandle, static_cast<int>(v_NewPriority));
#elif defined(__linux__)
		//No implementation exists
#endif
	}

	bool NativeThread::closeHandle(const ThreadHandle& ro_Handle) {
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [&ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
			return ro_Handle.m_HandleID == ro_InternalHandle.getCoriumID();
			});
		if (!ro_Handle.isValid() || itr->isHandleClosed()) return false;
#if defined(_WIN32)
		if (CloseHandle(itr->m_ThreadHandle)) {
			itr->m_IsValid = false;
			itr->m_IsClosed = true;
			return true;
		}
		return false;
#elif defined(__linux__)
		if (pthread_detach(itr->m_ThreadObject) == 0) {
			itr->m_IsValid = false;
			itr->m_IsClosed = true;
			return true;
		}
		return false;
#else
		return false;
#endif
	}

	ProcessorIdx NativeThread::getCurrentProcessorNumber() {
#if defined(_WIN32)
		return GetCurrentProcessorNumber();
#else
		return -1;
#endif
	}

	size_t NativeThread::getHardwareConcurrency() {
#if defined(_WIN32)
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		return sysInfo.dwNumberOfProcessors;
#elif defined(__linux__)
		return sysconf(_SC_NPROCESSORS_ONLN);
#else
		return 0;
#endif
	}

	size_t NativeThread::suspendThread(const ThreadHandle& ro_Handle) {
		if (!ro_Handle.isValid()) return false;
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [&ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
			return ro_Handle.m_HandleID == ro_InternalHandle.getCoriumID();
			});
#if defined(_WIN32)
		if (itr != m_Handles.end()) {
			itr->m_IsRunning = false;
			return SuspendThread(itr->m_ThreadHandle);
		}
		return -1;

#else
		return -1;
#endif
	}

	size_t NativeThread::resumeThread(const ThreadHandle& ro_Handle) {
		if (!ro_Handle.isValid()) return false;
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [&ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
			return ro_Handle.m_HandleID == ro_InternalHandle.getCoriumID();
			});
#if defined(_WIN32)
		if (itr != m_Handles.end()) {
			itr->m_IsRunning = true;
			return ResumeThread(itr->m_ThreadHandle);
		}
		return -1;
#else
		return -1;
#endif
	}

	bool NativeThread::duplicate(const ThreadHandle& ro_Handle, ThreadHandle& ro_Duplicate) {
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [&ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
			return ro_Handle.m_HandleID == ro_InternalHandle.getCoriumID();
			});
		if (itr == m_Handles.end() || itr->isHandleClosed()) return false;

		CPUThreadHandle duplicate;
		initHandle(duplicate);

		duplicate.m_IsRunning = itr->m_IsRunning;
		duplicate.m_IsValid = itr->m_IsValid;
		duplicate.m_CoriumThreadID = generateApiThreadID();
		duplicate.m_IsClosed = itr->m_IsClosed;
		duplicate.m_ThreadName = itr->m_ThreadName;

#if defined(_WIN32)
		HANDLE kDuplicate = nullptr;
		BOOL success = DuplicateHandle(GetCurrentProcess(), itr->m_ThreadHandle, GetCurrentProcess(), &kDuplicate, 0, FALSE, DUPLICATE_SAME_ACCESS);
		if (!success) return false;

		duplicate.m_CurrentThreadPriority = itr->m_CurrentThreadPriority;
		duplicate.m_ExitCode = itr->m_ExitCode;
		duplicate.m_ThreadHandle = kDuplicate;
		duplicate.m_ThreadID = itr->m_ThreadID;

		ro_Duplicate.m_HandleID = duplicate.m_CoriumThreadID;

		m_Handles.push_back(std::move(duplicate));
		return true;

#elif defined(__linux__)
		return false; //No duplication in linux
#endif
	}

	size_t NativeThread::joinThread(const ThreadHandle& ro_Handle) {
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [&ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
			return ro_Handle.m_HandleID == ro_InternalHandle.getCoriumID();
			});
		if (itr == m_Handles.end() || itr->isHandleClosed()) return 0;
#if defined(_WIN32)
		WaitForSingleObject(itr->m_ThreadHandle, INFINITE);
		DWORD exitCode;
		GetExitCodeThread(itr->m_ThreadHandle, &exitCode);
		return exitCode;
#elif defined(__linux__)
		return pthread_join(itr->m_ThreadObject, NULL);
#endif
	}

	bool NativeThread::terminateThread(const ThreadHandle& ro_Handle) {
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [&ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
			return ro_Handle.m_HandleID == ro_InternalHandle.getCoriumID();
			});
		if (itr == m_Handles.end() || itr->isHandleClosed()) return false;
#if defined(_WIN32)
		BOOL termination = TerminateThread(itr->m_ThreadHandle, 0);
		itr->m_IsRunning = !termination;
		return termination;
#elif defined(__linux__)
		int cancel = pthread_cancel(itr->threadObject);
		itr->m_IsRunning = !cancel;
		return cancel == 0;
#endif
	}

	size_t NativeThread::getThreadID(const ThreadHandle& ro_Handle) {
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [&ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
			return ro_Handle.m_HandleID == ro_InternalHandle.getCoriumID();
			});
		if (itr == m_Handles.end() || itr->isHandleClosed()) return 0;
		return itr->getCoriumID();
	}

	size_t NativeThread::getCurrentThreadID() {
		return getThreadID(this_platform_thread::t_Handle);
	}

	bool NativeThread::isAlive(const ThreadHandle& ro_Handle) {
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [&ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
			return ro_Handle.m_HandleID == ro_InternalHandle.getCoriumID();
			});
		if (itr == m_Handles.end() || itr->isHandleClosed()) return false;
		return itr->isRunning();
	}

	std::string NativeThread::getName(const ThreadHandle& ro_Handle) {
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [&ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
			return ro_Handle.m_HandleID == ro_InternalHandle.getCoriumID();
			});
		if (itr == m_Handles.end() || itr->isHandleClosed()) return "";
		return itr->getThreadName();
	}

	void NativeThread::waitOnAddress(const ParkHandle& ro_Permit) {
#if defined(_WIN32)
		int expected = ro_Permit.m_ParkingAddress.load(std::memory_order_acquire);
		WaitOnAddress(&const_cast<ParkHandle&>(ro_Permit).m_ParkingAddress, &expected, sizeof(int), INFINITE);
#elif defined(__linux__)
		int expected = ro_Permit.m_ParkingAddress.load(std::memory_order_acquire);
		futexWait(const_cast<std::atomic<int>*>(&ro_Permit.m_ParkingAddress), expected);
#endif
	}

	void NativeThread::wakeAllOnAddress(ParkHandle& ro_Permit) {
#if defined(_WIN32)
		WakeByAddressAll(&ro_Permit.m_ParkingAddress);
#elif defined(__linux__)
		futexWake(&ro_Permit.m_ParkingAddress, INT_MAX);
#endif
	}

	void NativeThread::wakeOnAddress(ParkHandle& ro_Permit) {
#if defined(_WIN32)
		WakeByAddressSingle(&ro_Permit.m_ParkingAddress);
#elif defined(__linux__)
		futexWake(&ro_Permit.m_ParkingAddress, 1);
#endif
	}

	void NativeThread::park() {
#if defined(_WIN32)
		int expected = this_platform_thread::t_ParkingPermit.m_ParkingAddress.load(std::memory_order_relaxed);
		WaitOnAddress(&this_platform_thread::t_ParkingPermit.m_ParkingAddress, &expected, sizeof(int), INFINITE);
#elif defined(__linux__)
		int expected =this_platform_thread::t_ParkingPermit.m_ParkingAddress.load(std::memory_order_relaxed);
		futexWait(const_cast<std::atomic<int>*>(&this_platform_thread::t_ParkingPermit.m_ParkingAddress), expected);
#endif
	}

	void NativeThread::unpark(ParkHandle& ro_Permit) {
		wakeAllOnAddress(ro_Permit); //Wrapper on wakeOnAddress
	}

}