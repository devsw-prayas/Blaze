#pragma once
#include "Corium.h"
#include "ThreadUtils.h"

namespace Corium::Core {

	class CORIUM_RUNTIME_API NativeThread {
	private:
		static bool isValidHandle(const ThreadHandle& ro_Handle) noexcept;

	public:
		static ThreadHandle createThread(
			ThreadLaunchDesc&& u_LaunchDesc, const ThreadAttrDesc& ro_ExecDesc) noexcept;
		static bool         detachThread(const ThreadHandle& ro_Handle) noexcept;
		static bool         closeHandle(const ThreadHandle& ro_Handle) noexcept;
		static ThreadHandle duplicateHandle(const ThreadHandle& ro_Handle) noexcept;

		static bool         isAlive(const ThreadHandle& ro_Handle) noexcept;
		static ProcessorIdx getThreadID(const ThreadHandle& ro_Handle) noexcept;

		static bool suspendThread(const ThreadHandle& ro_Handle) noexcept;
		static bool resumeThread(const ThreadHandle& ro_Handle) noexcept;
		static bool terminateThread(const ThreadHandle& ro_Handle) noexcept;
		static bool joinThread(const ThreadHandle& ro_Handle) noexcept;

		static void waitOnAddress(ParkHandle& ro_Permit, uint32_t expected) noexcept;
		static void wakeOnAddress(ParkHandle& ro_Permit) noexcept;
		static void wakeAllOnAddress(ParkHandle& ro_Permit) noexcept;

		template<typename T>
		static void waitOnAddressFor(ParkHandle& ro_Handle, T&& u_Duration) noexcept {
#ifdef _WIN32
			// TODO: timed WaitOnAddress implementation
#else
			// TODO: Linux (futex) implementation
#endif
		}
	};
}
