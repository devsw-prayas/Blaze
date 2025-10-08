#pragma once

#include "ThreadFactory.h"

namespace Corium::Factory {
	template<size_t StackSize = 0>
	class CORIUM DefaultThreadFactory final : IThreadFactory {
	public:
		Platform::ThreadHandle createThread(std::function<void()>&& u_Launch) override {
			Platform::NativeThreadOptions options{ "Sample " };
			options.StartSuspended(false);
			options.launchFunction(u_Launch);
			options.reservedStackSize(StackSize);

			Platform::NativeThreadAttributes attributes{};
			attributes.canDetach(false);
			return Platform::NativeThread::createThread(attributes, options);
		}
	};

	template<size_t StackSize = 0, int8_t Priority = 1, size_t CoreMask = 0> //THREAD_PRIORITY_ABOVE_NORMAL
	class CORIUM HighPriorityThreadFactory final: IThreadFactory {
	public:
		Platform::ThreadHandle createThread(std::function<void()>&& u_Launch) override {
			Platform::NativeThreadOptions options{ "Sample " };
			options.StartSuspended(false);
			options.launchFunction(u_Launch);
			options.reservedStackSize(StackSize);

			Platform::NativeThreadAttributes attributes{};
			attributes.canDetach(false);
			attributes.setLaunchPriority(Priority);
			if constexpr (CoreMask) attributes.groupAffinity(CoreMask);
			return Platform::NativeThread::createThread(attributes, options);
		}
	};

	template<size_t StackSize = 0, size_t CoreMask = 0> //THREAD_PRIORITY_ABOVE_NORMAL
	class CORIUM AffinityThreadFactory final : IThreadFactory {
	public:
		Platform::ThreadHandle createThread(std::function<void()>&& u_Launch) override {
			Platform::NativeThreadOptions options{ "Sample" };
			options.StartSuspended(false);
			options.launchFunction(u_Launch);
			options.reservedStackSize(StackSize);

			Platform::NativeThreadAttributes attributes{};
			attributes.canDetach(false);
			if constexpr (CoreMask) attributes.groupAffinity(CoreMask);
			return Platform::NativeThread::createThread(attributes, options);
		}
	};
}
	