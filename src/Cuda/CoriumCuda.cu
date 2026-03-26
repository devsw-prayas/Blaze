#include "Corium.h"
#include "CoriumCuda.h"

#define ALLOW_SYSCALL
#include "CoriumSyscalls.h"

namespace Corium::Cuda {
	static bool s_Available = false;
	static int s_DeviceId = -1;

	bool GPU::init() {
		int count = 0;
		cudaGetDeviceCount(&count);
		if (count == 0) return false;

		if (cudaSetDevice(0) != cudaSuccess) return false;
		s_DeviceId = 0;
		s_Available = true;
		return true;
	}

	bool GPU::isAvailable() {
		return s_Available;
	}
}
