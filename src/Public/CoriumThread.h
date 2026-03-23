#pragma once
#include "Corium.h"
#include "ThreadUtils.h"

namespace Corium::Core{
	
	class CORIUM_RUNTIME_API NativeThread {
		static ThreadHandle createThread(
			const ThreadLaunchDesc& ro_LaunchDesc, const ThreadAttrDesc& ro_ExecDesc) noexcept;
		static bool detachThread(ThreadHandle v_Handle) noexcept;
		static bool closeHandle(ThreadHandle v_Handle) noexcept;
		static ThreadHandle duplicateHandle(ThreadHandle v_handle) noexcept;

		static bool isAlive(ThreadHandle v_Handle) noexcept;
		static ProcessorIdx getThreadID(ThreadHandle v_Handle) noexcept;

		static bool suspendThread(ThreadHandle v_Handle) noexcept;
		static bool resumeThread(ThreadHandle v_Handle) noexcept;
		static bool terminateThread(ThreadHandle v_Handle) noexcept;
		static bool joinThread(ThreadHandle v_Handle) noexcept;

		static void waitOnAddress(ParkHandle& ro_Permit) noexcept;
		static void wakeOnAddress(ParkHandle& ro_Permit) noexcept;
		static void wakeAllOnAddress(ParkHandle& ro_Permit) noexcept;

		template<typename T>
		static void waitOnAddressFor(ParkHandle& ro_Handle, T&& u_Duration) noexcept {

		}
	};
}
