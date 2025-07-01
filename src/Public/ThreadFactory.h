#pragma once
#include <functional>
#include "Blaze.h"
#include "PlatformHandles.h"
namespace blaze{
	class Blaze CPUThreadFactory {
	public:
		virtual ~CPUThreadFactory() = default;
		virtual handles::THREAD_VARIANT createThread(const handles::CPUThreadOptions& options, std::function<void()>, bool useJThread) = 0;

		CPUThreadFactory(const CPUThreadFactory&) = delete;
		CPUThreadFactory& operator=(const CPUThreadFactory&) = delete;
		CPUThreadFactory(CPUThreadFactory&&) = default;
		CPUThreadFactory& operator=(CPUThreadFactory&&) = default;
	protected:
		CPUThreadFactory() = default;
	};

	class Blaze DefaultCPUThreadFactory : public CPUThreadFactory {
	public:
		handles::THREAD_VARIANT createThread(const handles::CPUThreadOptions& config,
			std::function<void()> workerFunc,
			bool useJThread) override;
	};

	class Blaze HighPriorityCPUThreadFactory : public CPUThreadFactory {
	public:
		handles::THREAD_VARIANT createThread(const handles::CPUThreadOptions& config,
			std::function<void()> workerFunc,
			bool useJThread) override;
	};

	class Blaze IOCPUThreadFactory : public CPUThreadFactory {
	public:
		handles::THREAD_VARIANT createThread(const handles::CPUThreadOptions& config,
			std::function<void()> workerFunc,
			bool useJThread) override;
	};

	class Blaze DebugCPUThreadFactory : public CPUThreadFactory {
	public:
		handles::THREAD_VARIANT createThread(const handles::CPUThreadOptions& config,
			std::function<void()> workerFunc,
			bool useJThread) override;
	};
}
