#include "Corium.h"
#include "CudaContextManager.h"

#include "CudaBootstrap.h"

#define ALLOW_SYSCALL
#include "CoriumSyscalls.h"
#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Corium::Cuda::Context {
	Utils::CudaContext ContextManager::createCudaContext(
		Utils::DeviceHandle v_Handle,
		Utils::ContextSchedulingFlags v_SchedFlag,
		Utils::ContextCreationFlags v_CreateFlag) {
		CORIUM_ASSERT(Internal::CUDA_DeviceRegistry::validateDevice(v_Handle.m_HandleValue));
		CUcontext context{};
		uint32_t flags = 0;
		if (v_CreateFlag == Utils::ContextCreationFlags::NONE) {
			flags = Internal::CUDA_InternalHelpers::toCudaContextScheduleFlags(v_SchedFlag);
		} else {
			flags = Internal::CUDA_InternalHelpers::toCudaContextScheduleFlags(v_SchedFlag) |
				Internal::CUDA_InternalHelpers::toCudaContextCreationFlags(v_CreateFlag);
		}

		Utils::CudaContext ctx;
		ctx.m_Handle = nullptr;
		CUresult result = cuCtxCreate_v2(&context, flags, Internal::CUDA_DeviceRegistry::s_Devices[v_Handle.m_HandleValue]);
		if (result == CUDA_SUCCESS) {
			ctx.m_Handle = context;
			return ctx;
		}
		CUDA_ERROR_TRAP(result)
		return ctx;
	}

	void ContextManager::setCurrentCudaContext(const Utils::CudaContext& ro_Context) {
		CORIUM_ASSERT(ro_Context.m_Handle);
		const CUresult result = cuCtxSetCurrent(static_cast<CUcontext>(ro_Context.m_Handle));
		if (result == CUDA_SUCCESS) return;

		CUDA_ERROR_TRAP(result)
	}

	Utils::CudaContext ContextManager::getCurrentCudaContext() {
		CUcontext context{};
		Utils::CudaContext ctx;
		const CUresult result = cuCtxGetCurrent(&context);
		ctx.m_Handle = nullptr;
		if (result == CUDA_SUCCESS) {
			ctx.m_Handle = context;
			return ctx;
		}
		CUDA_ERROR_TRAP(result)
		return ctx;
	}

	void ContextManager::pushCudaContext(const Utils::CudaContext& ro_Context) {
		CORIUM_ASSERT(ro_Context.m_Handle);
		CUresult result = cuCtxPushCurrent_v2(static_cast<CUcontext>(ro_Context.m_Handle));
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result)
	}

	Utils::CudaContext ContextManager::popCudaContext() {
		CUcontext context{};
		CUresult result = cuCtxPopCurrent_v2(&context);
		Utils::CudaContext ctx;
		ctx.m_Handle = nullptr;
		if (result == CUDA_SUCCESS) {
			ctx.m_Handle = context;
			return ctx;
		}

		CUDA_ERROR_TRAP(result)
		return ctx;
	}

	void ContextManager::cudaContextSynchronize() {
		CUresult result = cuCtxSynchronize();
		if (result == CUDA_SUCCESS) return;
		CUDA_ERROR_TRAP(result)
	}

	void ContextManager::destroyCudaContext(Utils::CudaContext& ro_Context) {
		CORIUM_ASSERT(ro_Context.m_Handle);
		CUresult result = cuCtxDestroy_v2(static_cast<CUcontext>(ro_Context.m_Handle));
		if (result == CUDA_SUCCESS) {
			ro_Context.m_Handle = nullptr;
			return;
		}
		CUDA_ERROR_TRAP(result)
	}
}