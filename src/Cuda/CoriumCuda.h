#pragma once

#ifdef CORIUM_CUDA_AVAILABLE
namespace Corium::Cuda {
	class CORIUM_RUNTIME_API GPU final {
	public:
		static bool init();
		static bool isAvailable();
	};
}

#endif