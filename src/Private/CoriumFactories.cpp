#include "Corium.h"
#include "CoriumFactories.h"

namespace Corium::Factory {
	template<const char* Name = "Default Thread", size_t StackSize = 0>
	Platform::ThreadHandle DefaultThreadFactory<Name, StackSize>::createThread(std::function<void()>&& u_Launch) {
		Platform::NativeThreadOptions options{ Name };
		options.StartSuspended(false);
		options.launchFunction(u_Launch);
		options.reservedStackSize(StackSize);

		Platform::NativeThreadAttributes attributes{};
		attributes.canDetach(false);
		return Platform::NativeThread::createThread(attributes, options);
	}

	template <const char* Name, size_t StackSize, int8_t Priority, size_t CoreMask>
	Platform::ThreadHandle HighPriorityThreadFactory<Name, StackSize, Priority, CoreMask>::createThread(std::function<void()>&& u_Launch) {
		Platform::NativeThreadOptions options{ Name };
		options.StartSuspended(false);
		options.launchFunction(u_Launch);
		options.reservedStackSize(StackSize);

		Platform::NativeThreadAttributes attributes{};
		attributes.canDetach(false);
		attributes.setLaunchPriority(Priority);
		if constexpr (CoreMask) attributes.groupAffinity(CoreMask);
		return Platform::NativeThread::createThread(attributes, options);
	}

	template <const char* Name, size_t StackSize, size_t CoreMask>
	Platform::ThreadHandle AffinityThreadFactory<Name, StackSize, CoreMask>::createThread(std::function<void()>&& u_Launch) {
		Platform::NativeThreadOptions options{ Name };
		options.StartSuspended(false);
		options.launchFunction(u_Launch);
		options.reservedStackSize(StackSize);

		Platform::NativeThreadAttributes attributes{};
		attributes.canDetach(false);
		if constexpr (CoreMask) attributes.groupAffinity(CoreMask);
		return Platform::NativeThread::createThread(attributes, options);
	}
}