#include "ThreadFactory.h"
#include <chrono>

namespace blaze {
	handles::THREAD_VARIANT DefaultCPUThreadFactory::createThread(const handles::CPUThreadOptions& config, std::function<void()> workerFunc, bool useJThread) {
		handles::CPUThreadOptions adjustedConfig = config;
		if (adjustedConfig.priority == 0) {
			adjustedConfig.priority = 0;
		}
		if (adjustedConfig.stackSize == 0) {
			adjustedConfig.stackSize = static_cast<size_t>(1024) * 1024;
		}
		if (adjustedConfig.name.empty()) {
			static int threadId = 0;
			adjustedConfig.name = "CPUWorker-" + std::to_string(threadId++);
		}

		if (adjustedConfig.numaNode == -1 && adjustedConfig.coreID != -1) {
			adjustedConfig.numaNode = handles::CPUThreadHandle::getNumaNodeForCore(adjustedConfig.coreID);
		}

		handles::THREAD_VARIANT thread;
#ifdef _WIN32
		if (useJThread) {
			thread = std::jthread(workerFunc);
		}
		else {
			thread = std::thread(workerFunc);
		}
#else
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		if (adjustedConfig.stackSize > 0) {
			pthread_attr_setstacksize(&attr, adjustedConfig.stackSize);
		}
		if (useJThread) {
			thread = std::jthread(workerFunc, &attr);
		}
		else {
			thread = std::thread(workerFunc, &attr);
		}
		pthread_attr_destroy(&attr);
#endif
		handles::CPUThreadHandle::configureThread(thread, adjustedConfig);
		return thread;
	}

	handles::THREAD_VARIANT HighPriorityCPUThreadFactory::createThread(const handles::CPUThreadOptions& config, std::function<void()> workerFunc, bool useJThread) {
		handles::CPUThreadOptions adjustedConfig = config;
		if (adjustedConfig.priority == 0) {
			adjustedConfig.priority = 2;
		}
		if (adjustedConfig.stackSize == 0) {
			adjustedConfig.stackSize = static_cast<size_t>(1024) * 1024 * 2;
		}
		if (adjustedConfig.name.empty()) {
			static int threadId = 0;
			adjustedConfig.name = "HighPriorityCPUWorker-" + std::to_string(threadId++);
		}

		if (adjustedConfig.numaNode == -1 && adjustedConfig.coreID != -1) {
			adjustedConfig.numaNode = handles::CPUThreadHandle::getNumaNodeForCore(adjustedConfig.coreID);
		}

		handles::THREAD_VARIANT thread;
#ifdef _WIN32
		if (useJThread) {
			thread = std::jthread(workerFunc);
		}
		else {
			thread = std::thread(workerFunc);
		}
#else
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		if (adjustedConfig.stackSize > 0) {
			pthread_attr_setstacksize(&attr, adjustedConfig.stackSize);
		}
		if (useJThread) {
			thread = std::jthread(workerFunc, &attr);
		}
		else {
			thread = std::thread(workerFunc, &attr);
		}
		pthread_attr_destroy(&attr);
#endif
		handles::CPUThreadHandle::configureThread(thread, adjustedConfig);
		return thread;
	}

	handles::THREAD_VARIANT IOCPUThreadFactory::createThread(const handles::CPUThreadOptions& config, std::function<void()> workerFunc, bool useJThread) {
		handles::CPUThreadOptions adjustedConfig = config;
		if (adjustedConfig.priority == 0) {
			adjustedConfig.priority = -1;
		}
		if (adjustedConfig.stackSize == 0) {
			adjustedConfig.stackSize = static_cast<size_t>(512) * 1024;
		}
		if (adjustedConfig.name.empty()) {
			static int threadId = 0;
			adjustedConfig.name = "IOCPUWorker-" + std::to_string(threadId++);
		}

		if (adjustedConfig.numaNode == -1 && adjustedConfig.coreID != -1) {
			adjustedConfig.numaNode = handles::CPUThreadHandle::getNumaNodeForCore(adjustedConfig.coreID);
		}

		handles::THREAD_VARIANT thread;
#ifdef _WIN32
		if (useJThread) {
			thread = std::jthread(workerFunc);
		}
		else {
			thread = std::thread(workerFunc);
		}
#else
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		if (adjustedConfig.stackSize > 0) {
			pthread_attr_setstacksize(&attr, adjustedConfig.stackSize);
		}
		if (useJThread) {
			thread = std::jthread(workerFunc, &attr);
		}
		else {
			thread = std::thread(workerFunc, &attr);
		}
		pthread_attr_destroy(&attr);
#endif
		handles::CPUThreadHandle::configureThread(thread, adjustedConfig);
		return thread;
	}

	handles::THREAD_VARIANT DebugCPUThreadFactory::createThread(const handles::CPUThreadOptions& config,
		std::function<void()> workerFunc,
		bool useJThread) {
		handles::CPUThreadOptions adjustedConfig = config;

		// Set debug-specific settings
		if (adjustedConfig.priority == 0) {
			adjustedConfig.priority = -1; // Below normal to avoid interference
		}
		if (adjustedConfig.stackSize == 0) {
			adjustedConfig.stackSize = static_cast<size_t>(1024) * 1024; // 1MB
		}
		if (adjustedConfig.name.empty()) {
			static int threadId = 0;
			adjustedConfig.name = "DebugCPUWorker-" + std::to_string(threadId++) + "-" +
				std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
		}

		// Determine NUMA node if not specified
		if (adjustedConfig.numaNode == -1 && adjustedConfig.coreID != -1) {
			adjustedConfig.numaNode = handles::CPUThreadHandle::getNumaNodeForCore(adjustedConfig.coreID);
		}

		// Log thread creation
		//TODO

		// Create the thread with the specified stack size
		handles::THREAD_VARIANT thread;
#ifdef _WIN32
		if (useJThread) {
			thread = std::jthread([workerFunc, name = adjustedConfig.name]() {
				//TODO : Log thread start
				workerFunc();
				//TODO : Log thread exit
				});
		}
		else {
			thread = std::thread([workerFunc, name = adjustedConfig.name]() {
				//TODO : Log thread start
				workerFunc();
				//TODO : Log thread exit
				});
		}
#else
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		if (adjustedConfig.stackSize > 0) {
			pthread_attr_setstacksize(&attr, adjustedConfig.stackSize);
		}
		if (useJThread) {
			thread = std::jthread([workerFunc, name = adjustedConfig.name]() {
				//TODO : Log thread start
				workerFunc();
				//TODO : Log thread exit
		}
		else {
			thread = std::thread([workerFunc, name = adjustedConfig.name]() {
				//TODO: Log thread start
				workerFunc();
				//TODO: Log thread exit
		}
		pthread_attr_destroy(&attr);
#endif
		handles::CPUThreadHandle::configureThread(thread, adjustedConfig);
		return thread;
	}
}