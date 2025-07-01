#pragma once
#include <string>
#include <thread>
#include <variant>

#ifdef _WIN32
#include <Windows.h>
#undef max
#undef min
#else
#include <pthread.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <numa.h>
#endif

namespace blaze::handles {
	using THREAD_VARIANT = std::variant<std::thread, std::jthread>;

	struct alignas(64) Blaze CPUThreadOptions {
		std::string name;
		int priority = -1;
		int coreID = -1;
		size_t stackSize = 0;
		int numaNode = -1;
	};

	// Helper class for platform - specific thread configuration
	class CPUThreadHandle {
	public:
		// Get the number of available CPU cores
		static int getNumCores() {
#ifdef _WIN32
			SYSTEM_INFO sysInfo;
			GetSystemInfo(&sysInfo);
			return static_cast<int>(sysInfo.dwNumberOfProcessors);
#else
			return sysconf(_SC_NPROCESSORS_ONLN);
#endif
		}

		// Get the NUMA node for a given core (returns -1 if not supported)
		static int getNumaNodeForCore(int coreId) {
#ifdef _WIN32
			UCHAR nodeNumber;
			if (GetNumaProcessorNode(static_cast<UCHAR>(coreId), &nodeNumber) == 0) {
				return -1; // Failed to get NUMA node
			}
			return static_cast<int>(nodeNumber);
#else
			if (numa_available() == -1) {
				return -1; // NUMA not supported
			}
			return numa_node_of_cpu(coreId);
#endif
		}

		// Configure the thread with the given settings
		static void configureThread(THREAD_VARIANT& thread, const CPUThreadOptions& config) {
			// Use std::visit to handle both std::thread and std::jthread
			std::visit([&](auto& t) {
				// Get the native handle
#ifdef _WIN32
				HANDLE handle = t.native_handle();
#else
				pthread_t handle = t.native_handle();
#endif

				// Set thread priority
				setPriority(handle, config.priority);

				// Set CPU affinity
				if (config.coreID >= 0) {
					setAffinity(handle, config.coreID);
				}

				// Set thread name
				if (!config.name.empty()) {
					setThreadName(handle, config.name);
				}
				}, thread);
		}

		// Allocate memory on a specific NUMA node (returns nullptr if not supported)
		static void* allocateOnNumaNode(size_t size, int nodeId) {
#ifdef _WIN32
			if (nodeId < 0) {
				return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			}
			return VirtualAllocExNuma(GetCurrentProcess(), nullptr, size,
				MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE, nodeId);
#else
			if (numa_available() == -1 || nodeId < 0) {
				return malloc(size);
			}
			return numa_alloc_onnode(size, nodeId);
#endif
		}

		static int getNumaNodeCount() {
#ifdef _WIN32
			ULONG highest_node;
			if (!GetNumaHighestNodeNumber(&highest_node)) {
				return -1;  // Error
			}
			return static_cast<int>(highest_node + 1);
#else
			if (numa_available() < 0) return -1;
			return numa_max_node() + 1;
#endif
		}

	private:
		// Set thread priority
		static void setPriority(auto handle, int priority) {
#ifdef _WIN32
			int winPriority;
			if (priority > 0) {
				winPriority = THREAD_PRIORITY_ABOVE_NORMAL;
			}
			else if (priority < 0) {
				winPriority = THREAD_PRIORITY_BELOW_NORMAL;
			}
			else {
				winPriority = THREAD_PRIORITY_NORMAL;
			}
			if (!SetThreadPriority(handle, winPriority)) {
				//TODO....
			}
#else
			struct sched_param param;
			int policy = SCHED_OTHER;
			param.sched_priority = 0;

			if (priority > 0) {
				policy = SCHED_FIFO;
				param.sched_priority = 50; // High priority (1-99 for SCHED_FIFO)
			}
			else if (priority < 0) {
				if (nice(10) == -1) {
					//TODO...
				}
			}

			if (pthread_setschedparam(handle, policy, &param) != 0) {
				//TODO...
			}
#endif
		}

		// Set CPU affinity
		static void setAffinity(auto handle, int coreId) {
			int numCores = getNumCores();
			if (coreId >= numCores) {
				//TODO
			}

#ifdef _WIN32
			DWORD_PTR affinityMask = 1ULL << coreId;
			if (!SetThreadAffinityMask(handle, affinityMask)) {
				//TODO
			}
#else
			cpu_set_t cpuset;
			CPU_ZERO(&cpuset);
			CPU_SET(coreId, &cpuset);
			if (pthread_setaffinity_np(handle, sizeof(cpu_set_t), &cpuset) != 0) {
				//TODO....
			}
#endif
		}

		// Set thread name
		static void setThreadName(auto handle, const std::string& name) {
#ifdef _WIN32
			std::wstring wName(name.begin(), name.end());
			SetThreadDescription(handle, wName.c_str());
#else
			pthread_setname_np(handle, name.c_str());
#endif
		}
	};
}