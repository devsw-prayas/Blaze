#include "Corium.h"

#define ALLOW_SYSCALL
#include "CoriumSyscalls.h"
#include "CoriumCompiler.h"
#include "CoriumDiagnostics.h"
#include "CoriumThread.h"
#include "CoriumMemoryHandler.h"
#include "InternalUtils.h"

#if CORIUM_COMPILER_MSVC
#pragma comment(lib, "synchronization.lib")
#endif

namespace Corium::Core {
	using namespace Corium::Core::Atomics;
	using namespace Corium::Memory::Literals;

	// Internal constants

	constexpr size_t MAX_CORIUM_THREADS = 256;
	constexpr size_t INVALID_SLOT = static_cast<size_t>(-1);

	// LaunchContext
	//
	// Bump-allocated from GeneralAllocator[0], heap-lifetime.
	// Moved into by createThread, destroyed by WinThreadThunk after the
	// closure returns. Owns the Closure<void()> for the thread's lifetime.

	struct alignas(16) LaunchContext final {
		Closure<void()>                       m_Closure;
		Memory::Allocators::GeneralAllocator* m_Allocator;
		size_t                                m_Slot;
		size_t								  m_TlsSize;

		LaunchContext(Closure<void()>&& u_Closure,
			Memory::Allocators::GeneralAllocator* p_Allocator, size_t v_Slot, size_t v_TlsSize) noexcept
			: m_Closure(std::move(u_Closure))
			, m_Allocator(p_Allocator)
			, m_Slot(v_Slot)
			, m_TlsSize(v_TlsSize) {}
	};

	// RegistryEntry
	//
	// One cache-line per thread slot. m_TokenMask is a bitmask of outstanding
	// access tokens — allows multiple handles to a single OS thread (e.g.
	// duplicate). m_State is stored atomically to allow the thread itself to
	// mark RUNNING/SEALED from the thunk without a data race.

	struct alignas(64) RegistryEntry final {
		HANDLE                          m_OsHandle{ INVALID_HANDLE_VALUE };
		DWORD                           m_OsThreadId{ 0 };
		CORIUM_MAYBE_UNUSED uint32_t    m_Pad0{ 0 };
		uint64_t                        m_Generation{ 0 };
		Atomic::AtomicValue64<uint64_t> m_TokenMask{};
		Atomic::AtomicValue32<uint32_t> m_State{};
		uint32_t						m_NumaNode{};

		RegistryEntry() noexcept {
			m_State.store(static_cast<uint32_t>(ThreadState::REAPED), MemoryOrder::RELAXED);
		}

		size_t allocateToken() noexcept {
			for (;;) {
				uint64_t v_Mask = m_TokenMask.load(MemoryOrder::RELAXED);
				uint64_t v_Free = ~v_Mask;
				if (v_Free == 0u) return INVALID_SLOT;

				unsigned long v_Bit = 0;
#if CORIUM_COMPILER_MSVC
				_BitScanForward64(&v_Bit, v_Free);
#else
				// TODO: Linux (GCC/Clang __builtin_ctzll) implementation
#endif

				uint64_t v_Desired = v_Mask | (1ull << v_Bit);
				uint64_t v_Prev = v_Mask;
				CORIUM_UNUSED(m_TokenMask.compareExchange(
					&v_Mask, v_Desired,
					MemoryOrder::ACQ_REL, MemoryOrder::ACQUIRE));
				if (v_Mask == v_Prev) return static_cast<size_t>(v_Bit);
			}
		}

		void releaseToken(size_t v_Token) noexcept {
			uint64_t v_Bit = 1ull << v_Token;
			for (;;) {
				uint64_t v_Mask = m_TokenMask.load(MemoryOrder::RELAXED);
				uint64_t v_Desired = v_Mask & ~v_Bit;
				uint64_t v_Prev = v_Mask;
				CORIUM_UNUSED(m_TokenMask.compareExchange(
					&v_Mask, v_Desired,
					MemoryOrder::ACQ_REL, MemoryOrder::ACQUIRE));
				if (v_Mask == v_Prev) return;
			}
		}

		bool hasOutstandingTokens() const noexcept {
			return m_TokenMask.load(MemoryOrder::ACQUIRE) != 0u;
		}

		bool hasToken(size_t v_Token) const noexcept {
			if (v_Token >= 64u) return false;
			const uint64_t v_Bit = 1ull << v_Token;
			return (m_TokenMask.load(MemoryOrder::ACQUIRE) & v_Bit) != 0u;
		}

		ThreadState state() const noexcept {
			return static_cast<ThreadState>(m_State.load(MemoryOrder::ACQUIRE));
		}

		void setState(ThreadState v_State) noexcept {
			m_State.store(static_cast<uint32_t>(v_State), MemoryOrder::RELEASE);
		}
	};

	CORIUM_STATIC_ASSERT(sizeof(RegistryEntry) <= 64,
		"RegistryEntry must fit in one cache line");

	namespace {
		// Global thread registry — process-lifetime, zero-initialized.

		RegistryEntry g_Registry[MAX_CORIUM_THREADS];
		size_t findFreeSlot() noexcept {
			for (size_t i = 0; i < MAX_CORIUM_THREADS; ++i) {
				if (g_Registry[i].state() == ThreadState::REAPED &&
					!g_Registry[i].hasOutstandingTokens()) {
					return i;
				}
			}
			return INVALID_SLOT;
		}
	}

	bool NativeThread::isValidHandle(const ThreadHandle& ro_Handle) noexcept {
		const size_t v_Slot = ro_Handle.m_ThreadId;
		if (v_Slot >= MAX_CORIUM_THREADS) return false;
		const RegistryEntry& r_Entry = g_Registry[v_Slot];
		if (r_Entry.m_Generation != ro_Handle.m_Generation) return false;
		if (r_Entry.state() == ThreadState::REAPED) return false;
		if (!r_Entry.hasToken(ro_Handle.m_AccessToken)) return false;
		return true;
	}

	// Internal::ThreadLaunchHelper
	//
	// Forward-declared in ThreadUtils.h, defined here. Owns the Win32 thunk
	// so that DWORD/WINAPI never appear in the public header.
	// Friended by ThreadHandle for private constructor access.

	namespace Internal {
		struct ThreadLaunchHelper final {
			static DWORD WINAPI WinThreadThunk(void* p_Raw) noexcept;
		};
	}

	DWORD WINAPI Internal::ThreadLaunchHelper::WinThreadThunk(void* p_Raw) noexcept {
#ifdef _WIN32
		auto* p_Ctx = static_cast<LaunchContext*>(p_Raw);
		const size_t v_Slot = p_Ctx->m_Slot;

		g_Registry[v_Slot].setState(ThreadState::RUNNING);

		// Populate TLS with this thread's handle and a fresh park permit.
		this_thread::t_Handle = ThreadHandle(
			v_Slot,
			g_Registry[v_Slot].m_Generation,
			0u,
			ThreadState::RUNNING);
		this_thread::t_Permit = ParkHandle{ 0u };

		// Carve this thread's TLS slice and initialize the thread_local allocator.
		// VirtualSegment is heap-allocated from the same GeneralAllocator as LaunchContext —
		// one-time cost, irrelevant next to the kernel thread creation call.
		const uint32_t v_NumaNode = g_Registry[v_Slot].m_NumaNode;
		void* p_TlsMem = Memory::Internal::AtomicAllocators::instance()
			.s_TlsAllocator[v_NumaNode].load(MemoryOrder::ACQUIRE)->allocate(p_Ctx->m_TlsSize);

		auto* p_TlsSegment = p_Ctx->m_Allocator->emplace<Memory::VirtualSegment>(
			p_TlsMem, p_Ctx->m_TlsSize, 0u, static_cast<uint8_t>(v_NumaNode));
		this_thread::t_ThreadLocalAllocator.init(p_TlsSegment);

		// Launch from launch address
		p_Ctx->m_Closure();

		g_Registry[v_Slot].setState(ThreadState::SEALED);

		auto* p_Alloc = p_Ctx->m_Allocator;
		p_Alloc->deallocate(p_TlsSegment, sizeof(Memory::VirtualSegment));
		p_Ctx->~LaunchContext();
		p_Alloc->deallocate(p_Ctx, sizeof(LaunchContext));

		// TODO Frame
		return 0;
#else
		// TODO: Linux implementation
		return 0;
#endif
	}

	// NativeThread — method implementations

	ThreadHandle NativeThread::createThread(ThreadLaunchDesc&& u_LaunchDesc, const ThreadAttrDesc& ro_ExecDesc) noexcept {
		CORIUM_ASSERT(u_LaunchDesc.m_State == DescriptorState::FROZEN
			&& "ThreadLaunchDesc must be validated (call validate()) before createThread");
		CORIUM_ASSERT(ro_ExecDesc.m_State == DescriptorState::FROZEN
			&& "ThreadAttrDesc must be validated (call validate()) before createThread");

		const size_t v_Slot = findFreeSlot();
		if (v_Slot == INVALID_SLOT) return ThreadHandle::getInvalidThread();

		RegistryEntry& r_Entry = g_Registry[v_Slot];

		const size_t v_Token = r_Entry.allocateToken();
		if (v_Token == INVALID_SLOT) return ThreadHandle::getInvalidThread();

		auto* p_Alloc = &Memory::Internal::AllocatorRegistry::s_GeneralAllocator[0];

		auto* p_Ctx = p_Alloc->emplace<LaunchContext>(
			std::move(u_LaunchDesc.m_StartPoint),
			p_Alloc,
			v_Slot,
			u_LaunchDesc.m_VaSize);

		if (!p_Ctx) {
			r_Entry.releaseToken(v_Token);
			return ThreadHandle::getInvalidThread();
		}

#ifdef _WIN32
		const DWORD v_CreateFlags = u_LaunchDesc.m_IsPreSuspended ? CREATE_SUSPENDED : 0u;

		const uint32_t attrCount = (ro_ExecDesc.m_Mask != 0 ? 1u : 0u)
			+ (ro_ExecDesc.m_SupportsIdealProcessor ? 1u : 0u);

		LPPROC_THREAD_ATTRIBUTE_LIST attrList = nullptr;
		LPVOID attrListBuf = nullptr;

		if (attrCount > 0) {
			SIZE_T attrListSize = 0;
			InitializeProcThreadAttributeList(nullptr, attrCount, 0, &attrListSize);
			attrListBuf = HeapAlloc(GetProcessHeap(), 0, attrListSize);
			if (!attrListBuf) {
				p_Ctx->~LaunchContext();
				p_Alloc->deallocate(p_Ctx, sizeof(LaunchContext));
				r_Entry.releaseToken(v_Token);
				return ThreadHandle::getInvalidThread();
			}
			attrList = static_cast<LPPROC_THREAD_ATTRIBUTE_LIST>(attrListBuf);
			InitializeProcThreadAttributeList(attrList, attrCount, 0, &attrListSize);

			if (ro_ExecDesc.m_Mask != 0) {
				GROUP_AFFINITY groupAffinity{};
				groupAffinity.Mask = ro_ExecDesc.m_Mask;
				groupAffinity.Group = static_cast<WORD>(ro_ExecDesc.m_GroupId);
				UpdateProcThreadAttribute(attrList, 0, PROC_THREAD_ATTRIBUTE_GROUP_AFFINITY,
					&groupAffinity, sizeof(groupAffinity), nullptr, nullptr);
			}
			if (ro_ExecDesc.m_SupportsIdealProcessor) {
				PROCESSOR_NUMBER procNum{};
				procNum.Group = static_cast<WORD>(ro_ExecDesc.m_GroupId);
				procNum.Number = static_cast<BYTE>(ro_ExecDesc.m_IdealProcessor);
				UpdateProcThreadAttribute(attrList, 0, PROC_THREAD_ATTRIBUTE_IDEAL_PROCESSOR,
					&procNum, sizeof(procNum), nullptr, nullptr);
			}
		}

		DWORD  v_OsThreadId = 0;
		HANDLE v_OsHandle = CreateRemoteThreadEx(
			GetCurrentProcess(),
			nullptr,
			CORIUM_DEFAULT_OS_TLS_SIZE * 1_MiB,
			Internal::ThreadLaunchHelper::WinThreadThunk,
			p_Ctx,
			v_CreateFlags,
			attrList,
			&v_OsThreadId);

		if (attrList) {
			DeleteProcThreadAttributeList(attrList);
			HeapFree(GetProcessHeap(), 0, attrListBuf);
		}

		if (!v_OsHandle || v_OsHandle == INVALID_HANDLE_VALUE) {
			p_Ctx->~LaunchContext();
			p_Alloc->deallocate(p_Ctx, sizeof(LaunchContext));
			r_Entry.releaseToken(v_Token);
			return ThreadHandle::getInvalidThread();
		}

		// Thread priority.
		int v_WinPriority = Corium::Internal::toWin32Priority(ro_ExecDesc.m_ThreadPriority);
		SetThreadPriority(v_OsHandle, v_WinPriority);

		// Thread name (debugger-visible).
		if (u_LaunchDesc.m_Name && u_LaunchDesc.m_Name[0] != '\0') {
			wchar_t v_WideName[256] = {};
			for (size_t i = 0; i < 255 && u_LaunchDesc.m_Name[i]; ++i)
				v_WideName[i] = static_cast<wchar_t>(u_LaunchDesc.m_Name[i]);
			SetThreadDescription(v_OsHandle, v_WideName);
		}

		// Commit registry entry.
		r_Entry.m_OsHandle = v_OsHandle;
		r_Entry.m_OsThreadId = v_OsThreadId;
		r_Entry.m_NumaNode = ro_ExecDesc.m_NumaNode;
		++r_Entry.m_Generation;
		r_Entry.setState(ThreadState::CREATED);

		// Perfrom cleanup

#else
		// TODO: Linux (pthreads) implementation
		p_Ctx->~LaunchContext();
		p_Alloc->deallocate(p_Ctx, sizeof(LaunchContext));
		r_Entry.releaseToken(v_Token);
		return ThreadHandle::getInvalidThread();
#endif

		// TODO Kill me for this TLS upgrade
		return { v_Slot, r_Entry.m_Generation, v_Token, ThreadState::CREATED };
	}

	bool NativeThread::detachThread(const ThreadHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return false;
		RegistryEntry& r_Entry = g_Registry[ro_Handle.m_ThreadId];

		r_Entry.releaseToken(ro_Handle.m_AccessToken);

		// If no other handles reference this slot, close the OS handle.
		if (!r_Entry.hasOutstandingTokens()) {
#ifdef _WIN32
			CloseHandle(r_Entry.m_OsHandle);
			r_Entry.m_OsHandle = INVALID_HANDLE_VALUE;
			r_Entry.m_OsThreadId = 0;
			// State stays as-is — the thread may still be running.
#else
			// TODO: Linux implementation
#endif
		}
		return true;
	}

	bool NativeThread::closeHandle(const ThreadHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return false;
		RegistryEntry& r_Entry = g_Registry[ro_Handle.m_ThreadId];

		r_Entry.releaseToken(ro_Handle.m_AccessToken);

		if (!r_Entry.hasOutstandingTokens()) {
#ifdef _WIN32
			if (r_Entry.m_OsHandle != INVALID_HANDLE_VALUE) {
				CloseHandle(r_Entry.m_OsHandle);
				r_Entry.m_OsHandle = INVALID_HANDLE_VALUE;
				r_Entry.m_OsThreadId = 0;
			}
#else
			// TODO: Linux implementation
#endif
			r_Entry.setState(ThreadState::REAPED);
		}
		return true;
	}

	ThreadHandle NativeThread::duplicateHandle(const ThreadHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return ThreadHandle::getInvalidThread();
		RegistryEntry& r_Entry = g_Registry[ro_Handle.m_ThreadId];

		const size_t v_NewToken = r_Entry.allocateToken();
		if (v_NewToken == INVALID_SLOT) return ThreadHandle::getInvalidThread();

		return { ro_Handle.m_ThreadId, r_Entry.m_Generation,v_NewToken, r_Entry.state() };
	}

	bool NativeThread::isAlive(const ThreadHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return false;
#ifdef _WIN32
		const RegistryEntry& r_Entry = g_Registry[ro_Handle.m_ThreadId];
		if (r_Entry.m_OsHandle == INVALID_HANDLE_VALUE) return false;

		DWORD v_ExitCode = 0;
		if (!GetExitCodeThread(r_Entry.m_OsHandle, &v_ExitCode)) return false;
		return v_ExitCode == STILL_ACTIVE;
#else
		// TODO: Linux implementation
		return false;
#endif
	}

	ProcessorIdx NativeThread::getThreadID(const ThreadHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return 0;
#ifdef _WIN32
		return static_cast<ProcessorIdx>(g_Registry[ro_Handle.m_ThreadId].m_OsThreadId);
#else
		// TODO: Linux implementation
		return 0;
#endif
	}

	bool NativeThread::suspendThread(const ThreadHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return false;
#ifdef _WIN32
		const HANDLE v_OsHandle = g_Registry[ro_Handle.m_ThreadId].m_OsHandle;
		if (v_OsHandle == INVALID_HANDLE_VALUE) return false;
		return ::SuspendThread(v_OsHandle) != static_cast<DWORD>(-1);
#else
		// TODO: Linux implementation
		return false;
#endif
	}

	bool NativeThread::resumeThread(const ThreadHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return false;
#ifdef _WIN32
		const HANDLE v_OsHandle = g_Registry[ro_Handle.m_ThreadId].m_OsHandle;
		if (v_OsHandle == INVALID_HANDLE_VALUE) return false;
		return ::ResumeThread(v_OsHandle) != static_cast<DWORD>(-1);
#else
		// TODO: Linux implementation
		return false;
#endif
	}

	bool NativeThread::terminateThread(const ThreadHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return false;
#ifdef _WIN32
		RegistryEntry& r_Entry = g_Registry[ro_Handle.m_ThreadId];
		if (r_Entry.m_OsHandle == INVALID_HANDLE_VALUE) return false;

		const bool v_Ok = TerminateThread(r_Entry.m_OsHandle, 0) != FALSE;
		if (v_Ok) r_Entry.setState(ThreadState::SEALED);
		return v_Ok;
#else
		// TODO: Linux implementation
		return false;
#endif
	}

	bool NativeThread::joinThread(const ThreadHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return false;
#ifdef _WIN32
		const HANDLE v_OsHandle = g_Registry[ro_Handle.m_ThreadId].m_OsHandle;
		if (v_OsHandle == INVALID_HANDLE_VALUE) return false;
		return WaitForSingleObject(v_OsHandle, INFINITE) == WAIT_OBJECT_0;
#else
		// TODO: Linux implementation
		return false;
#endif
	}

	uint32_t NativeThread::getNumaNode(const ThreadHandle& ro_Handle) noexcept {
		if (!isValidHandle(ro_Handle)) return UINT32_MAX;
		return g_Registry[ro_Handle.m_ThreadId].m_NumaNode;
	}

	void NativeThread::waitOnAddress(ParkingSupport& ro_Support, uint32_t expected) noexcept {
#ifdef _WIN32
		WaitOnAddress(
			ro_Support.data(),
			&expected,
			sizeof(uint32_t),
			INFINITE
		);
#else
		// futex(..., FUTEX_WAIT, expected, ...)
#endif
	}

	void NativeThread::wakeOnAddress(ParkingSupport& ro_Support) noexcept {
#ifdef _WIN32
		ro_Support.store(1u, MemoryOrder::RELEASE);
		WakeByAddressSingle(ro_Support.data());
#else
		// TODO: Linux (futex) implementation
#endif
	}

	void NativeThread::wakeAllOnAddress(ParkingSupport& ro_Support) noexcept {
#ifdef _WIN32
		ro_Support.store(1u, MemoryOrder::RELEASE);
		WakeByAddressAll(ro_Support.data());
#else
		// TODO: Linux (futex) implementation
#endif
	}

	void NativeThread::waitOnAddressFor(ParkingSupport& ro_Support, Chrono::Instant v_Deadline) noexcept {
#ifdef _WIN32
		const int64_t v_Ms = v_Deadline.remainingMilliseconds();
		const DWORD v_Timeout = (v_Ms <= 0) ? 0u
			: static_cast<DWORD>(v_Ms < 0xFFFFFFFELL ? v_Ms : 0xFFFFFFFEu);
		uint32_t v_Expected = 0u;
		WaitOnAddress(ro_Support.data(), &v_Expected, sizeof(uint32_t), v_Timeout);
#else
		// TODO: Linux futex with timeout
		(void) ro_Support;
		(void) v_Deadline;
#endif
	}
}