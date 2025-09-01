#include "Corium.h"
#include "ThreadPlatform.h"

namespace Corium::Platform {
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
		auto l_TlsInit = [opaqueHandle](const std::function<void()>& v_callable) {
			this_platform_thread::t_Handle = opaqueHandle;
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
		threadHandle.m_ThreadHandle = thread;
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
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
			return ro_Handle.m_HandleID == ro_InternalHandle.getCoriumID();
			});

		if (itr != m_Handles.end()) {
#if defined(_WIN32)
			CloseHandle(itr->m_ThreadHandle);
			itr = m_Handles.erase(itr);
			ro_Handle.m_HandleID = INVALID_HANDLE;
			return true;
#elif defined(__linux__)
			pthread_detach(itr->m_ThreadHandle);
			itr = m_Handles.erase(itr);
			ro_Handle.m_HandleID = INVALID_HANDLE;
			return true;
#endif
		}
		return false;
	}

	bool NativeThread::setPriority(const ThreadHandle& ro_Handle, Priority v_NewPriority) {
		if (!ro_Handle.isValid()) return false;
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
			return ro_Handle.m_HandleID == ro_InternalHandle.getCoriumID();
			});
		if (itr == m_Handles.end()) return false;

#if defined(_WIN32)
		return SetThreadPriority(itr->m_ThreadHandle, static_cast<int>(v_NewPriority));
#elif defined(__linux__)
		//No implementation exists
#endif
	}

	bool NativeThread::closeHandle(CPUThreadHandle& ro_Handle) {
		if (!ro_Handle.m_IsValid) return false;
#if defined(_WIN32)
		if (CloseHandle(ro_Handle.m_ThreadHandle)) {
			ro_Handle.m_IsValid = false;
			ro_Handle.m_IsClosed = true;
			return true;
		}
		return false;
#elif defined(__linux__)
		if (pthread_detach(ro_Handle.m_ThreadHandle) == 0) {
			ro_Handle.m_IsValid = false;
			ro_Handle.m_IsClosed = true;
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
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
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
		auto itr = std::find_if(m_Handles.begin(), m_Handles.end(), [ro_Handle](const CPUThreadHandle& ro_InternalHandle) {
			return ro_Handle.m_HandleID == ro_InternalHandle.getCoriumID();
			});
#if defined(_WIN32)
		if (itr != m_Handles.end()) {
			itr->m_IsRunning = false;
			return ResumeThread(itr->m_ThreadHandle);
		}
		return -1;
#else
		return -1;
#endif
	}
}