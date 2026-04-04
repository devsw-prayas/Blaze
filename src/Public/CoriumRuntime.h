#pragma once

#include "Corium.h"
#include "CoriumMemoryHandler.h"
#include "CoriumAddrSpace.h"
#include "CoriumEnvironment.h"

#ifdef CORIUM_CUDA_AVAILABLE

#endif

namespace Corium {
	class CORIUM_RUNTIME_API CoriumRuntime final {
		static CORIUM_FORCEINLINE bool s_IsInit = false;
		static void CORIUM_FORCEINLINE initRuntime() {
			if (s_IsInit) return;

			// Probe CPU topology first -node count drives allocator init.
			Environment::EnvironmentProbe::init();

			const uint32_t detectedNodes = Environment::EnvironmentProbe::getCpuInfo().m_NumaNodeCount;
			const uint32_t rawNodes      = detectedNodes > 0 ? detectedNodes : 1u;
			const uint32_t nodeCount     = rawNodes < Memory::Internal::MAX_NUMA_NODES
			                             ? rawNodes
			                             : Memory::Internal::MAX_NUMA_NODES;

			// Reserve all per-node VA regions.
			Memory::Internal::init();

			// Wire allocators to their node VA regions.
			Memory::Internal::AllocatorRegistry::initRegistry(nodeCount);

			//If GPU capabilities are enabled in the build
#ifdef CORIUM_CUDA_AVAILABLE
#endif

			s_IsInit = true;
		}
	};
}
