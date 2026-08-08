#include "Corium.h"
#include "CudaBootstrap.h"
#include "CoriumCompiler.h"
#include "CoriumDiagnostics.h"

#define ALLOW_SYSCALL
#include "CoriumSyscalls.h"
#define ALLOW_HELPERS
#include "InternalUtils.h"

namespace Corium::Cuda {
	bool Bootstrap::CudaDriver::initCuda() {
		CUresult result = cuInit(0);

		if (result == CUDA_ERROR_NO_DEVICE)
			return false;

		const char* errorStr = nullptr;
		if (result != CUDA_SUCCESS) {
			cuGetErrorString(result, &errorStr);
			CORIUM_ASSERT(false && errorStr);
			CORIUM_TRAP();
		}
		// ADD other handling behavior later TODO

		// Get all CUDA Devices
		int count = 0;
		result = cuDeviceGetCount(&count);
		if (result != CUDA_SUCCESS) {
			cuGetErrorString(result, &errorStr);
			CORIUM_ASSERT(false && errorStr);
			CORIUM_TRAP();
		}
		Internal::CUDA_DeviceRegistry::s_DeviceCount = count > Internal::CUDA_DeviceRegistry::MAX_DEVICE_COUNT ? Internal::CUDA_DeviceRegistry::MAX_DEVICE_COUNT : count;
		for (int i = 0; i < Internal::CUDA_DeviceRegistry::s_DeviceCount; i++) {
			result = cuDeviceGet(&Internal::CUDA_DeviceRegistry::s_Devices[i], i);
			if (result != CUDA_SUCCESS) {
				cuGetErrorString(result, &errorStr);
				CORIUM_ASSERT(false && errorStr);
				CORIUM_TRAP();
			}
		}

		return true;
	}

	uint32_t Bootstrap::CudaDriver::getCudaDriverVersion() {
		int version = 0;
		CUresult result = cuDriverGetVersion(&version);
		if (result == CUDA_SUCCESS)
			return static_cast<uint32_t>(version);

		CUDA_ERROR_TRAP(result);
		return 0;
	}

	int Bootstrap::CudaDeviceManager::getCudaDeviceCount() {
		return Internal::CUDA_DeviceRegistry::s_DeviceCount;
	}

	Utils::DeviceHandle Bootstrap::CudaDeviceManager::getCudaDevice(int v_Ordinal) {
		CORIUM_ASSERT(Internal::CUDA_DeviceRegistry::validateDevice(v_Ordinal));
		Utils::DeviceHandle handle{};
		handle.m_HandleValue = v_Ordinal;
		return handle;
	}

	void Bootstrap::CudaDeviceManager::getCudaDeviceName(char* p_Name, uint32_t v_Len, Utils::DeviceHandle v_Handle) {
		CORIUM_ASSERT(p_Name != nullptr);
		CORIUM_ASSERT(v_Len > 0 && v_Len <= INT_MAX);
		CORIUM_ASSERT(Internal::CUDA_DeviceRegistry::validateDevice(v_Handle.m_HandleValue));
		CUresult result = cuDeviceGetName(p_Name, static_cast<int>(v_Len), Internal::CUDA_DeviceRegistry::s_Devices[v_Handle.m_HandleValue]);
		if (result == CUDA_SUCCESS) return;

		CUDA_ERROR_TRAP(result);
	}

	void Bootstrap::CudaDeviceManager::getCudaDeviceAttribute(int* p_Value, Utils::CudaDeviceAttribute v_Attr, Utils::DeviceHandle v_Handle) {
		CORIUM_ASSERT(p_Value != nullptr);
		CORIUM_ASSERT(Internal::CUDA_DeviceRegistry::validateDevice(v_Handle.m_HandleValue));
		CUresult result = cuDeviceGetAttribute(p_Value, Internal::CUDA_InternalHelpers::toCudaAttr(v_Attr), Internal::CUDA_DeviceRegistry::s_Devices[v_Handle.m_HandleValue]);
		if (result == CUDA_SUCCESS) return;

		CUDA_ERROR_TRAP(result);
	}

	size_t Bootstrap::CudaDeviceManager::getCudaDeviceTotalMemory(Utils::DeviceHandle v_Handle) {
		CORIUM_ASSERT(Internal::CUDA_DeviceRegistry::validateDevice(v_Handle.m_HandleValue));
		size_t mem = 0;
		CUresult result = cuDeviceTotalMem_v2(&mem, Internal::CUDA_DeviceRegistry::s_Devices[v_Handle.m_HandleValue]);
		if (result == CUDA_SUCCESS) return mem;
		CUDA_ERROR_TRAP(result);
		return 0;
	}

	Utils::DeviceUUID Bootstrap::CudaDeviceManager::getCudaDeviceUUID(Utils::DeviceHandle v_Handle) {
		CORIUM_ASSERT(Internal::CUDA_DeviceRegistry::validateDevice(v_Handle.m_HandleValue));
		CUuuid uuid{};
		Utils::DeviceUUID deviceUUID{};
		CUresult result = cuDeviceGetUuid_v2(&uuid, Internal::CUDA_DeviceRegistry::s_Devices[v_Handle.m_HandleValue]);
		if (result == CUDA_SUCCESS) {
			// Extremely dangerous, the struct layout must never be changed
			memcpy(&deviceUUID, uuid.bytes, 16);	
			return deviceUUID;
		}
		CUDA_ERROR_TRAP(result);
		return deviceUUID;
	}
}