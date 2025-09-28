#pragma once

#include "ThreadFactory.h"

namespace Corium::Factory {
	template<const char* Name = "Default Thread", size_t StackSize = 0>
	class DefaultThreadFactory final : IThreadFactory {
		Platform::ThreadHandle createThread(std::function<void()>&& u_Launch) override;
	};

	template<const char* Name = "Default Thread", size_t StackSize = 0, int8_t Priority = 1, size_t CoreMask> //THREAD_PRIORITY_ABOVE_NORMAL
	class HighPriorityThreadFactory final: IThreadFactory {
		Platform::ThreadHandle createThread(std::function<void()>&&) override;
	};

	template<const char* Name = "Default Thread", size_t StackSize = 0, size_t CoreMask> //THREAD_PRIORITY_ABOVE_NORMAL
	class AffinityThreadFactory final : IThreadFactory {
		Platform::ThreadHandle createThread(std::function<void()>&&) override;
	};
}
	