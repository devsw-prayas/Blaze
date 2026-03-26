#pragma once

#include "Corium.h"

#ifdef CORIUM_CUDA_AVAILABLE
#include "CoriumCuda.h"
#endif

#include "CoriumMemoryHandler.h"
#include "CoriumAddrSpace.h"

namespace Corium {
	class CORIUM_RUNTIME_API CoriumRuntime final {
		static CORIUM_FORCEINLINE bool s_IsInit = false;
		static void CORIUM_FORCEINLINE initRuntime() {
			if (s_IsInit) return;
			// Init all the internal engine infrastructure
			Memory::Internal::init();
			Memory::Internal::AllocatorRegistry::initRegistry();

			//If a GPU capabilities are enabled in the build
#ifdef CORIUM_CUDA_AVAILABLE
			Cuda::GPU::init();
#endif
		}
	};
}
