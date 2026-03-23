#include "Corium.h"

#define ALLOW_SYSCALL
#include "CoriumSyscalls.h"
#include "CoriumCompiler.h"
#include "CoriumDiagnostics.h"
#include "CoriumThread.h"

#ifdef CORIUM_COMPILER_MSVC
// TODO : Have to move to CMake
#pragma comment(lib, "synchronization.lib")
#endif

namespace Corium::Core {
	struct WinLaunchContext {
		void* userContext;
		Utils::FunctionView<void* (void*)> launchEntry;
	};

	static CORIUM_FORCEINLINE DWORD WINAPI WinThreadThunk(void* ctx) {
		auto* launch = static_cast<WinLaunchContext*>(ctx);
		CORIUM_UNUSED(launch->launchEntry(launch->userContext));
		return 0;
	}

	struct CORIUM_ALIGNAS(16) InvariantHandle final {
		HANDLE m_InternalHandle;
		Atomic::AtomicValue32<uint32_t> m_AccessCount;

		InvariantHandle() : m_InternalHandle(INVALID_HANDLE_VALUE), m_AccessCount(0) {}
		~InvariantHandle() = default;

		InvariantHandle(const InvariantHandle&) = default;
		InvariantHandle& operator=(const InvariantHandle&) = default;

		InvariantHandle(InvariantHandle&&) noexcept = default;
		InvariantHandle& operator=(InvariantHandle&&) noexcept = default;
	};
}
