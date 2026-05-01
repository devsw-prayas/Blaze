#include "Corium.h"
#include "CudaEvent.h"

#define ALLOW_SYSCALL
#include "CoriumSyscalls.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Corium::Cuda::Events {
	GpuEvent DeviceEvents::createEvent(uint32_t v_Flags) {
		CUevent hEvent;
		const CUresult result = cuEventCreate(&hEvent, static_cast<unsigned int>(v_Flags));

		if (result == CUDA_SUCCESS) {
			GpuEvent ro_Event;
			ro_Event.m_EventHandle = hEvent;
			return ro_Event;
		}

		CUDA_ERROR_TRAP(result);
		return GpuEvent{};
	}

	void DeviceEvents::destroyEvent(GpuEvent& ro_Event) {
		if (!ro_Event.isValid()) return;

		const CUresult result = cuEventDestroy(static_cast<CUevent>(ro_Event.m_EventHandle));
		if (result == CUDA_SUCCESS) {
			ro_Event.m_EventHandle = nullptr;
			return;
		}

		CUDA_ERROR_TRAP(result);
	}

	void DeviceEvents::recordEvent(const GpuEvent& ro_Event, const GpuStream& ro_Stream) {
		CORIUM_ASSERT(ro_Event.isValid());
		CORIUM_ASSERT(ro_Stream.isValid());

		const CUresult result = cuEventRecord(
			static_cast<CUevent>(ro_Event.m_EventHandle),
			static_cast<CUstream>(ro_Stream.m_StreamHandle)
		);

		if (result == CUDA_SUCCESS) return;

		CUDA_ERROR_TRAP(result);
	}

	void DeviceEvents::syncEvent(const GpuEvent& ro_Event) {
		CORIUM_ASSERT(ro_Event.isValid());

		const CUresult result = cuEventSynchronize(static_cast<CUevent>(ro_Event.m_EventHandle));
		if (result == CUDA_SUCCESS) return;

		CUDA_ERROR_TRAP(result);
	}

	bool DeviceEvents::queryEvent(const GpuEvent& ro_Event) {
		if (!ro_Event.isValid()) return true;

		const CUresult result = cuEventQuery(static_cast<CUevent>(ro_Event.m_EventHandle));
		if (result == CUDA_SUCCESS)         return true;
		if (result == CUDA_ERROR_NOT_READY) return false;

		CUDA_ERROR_TRAP(result);
		return false;
	}

	float DeviceEvents::elapsedTime(const GpuEvent& ro_Start, const GpuEvent& ro_End) {
		CORIUM_ASSERT(ro_Start.isValid());
		CORIUM_ASSERT(ro_End.isValid());

		float ms = 0.0f;
		const CUresult result = cuEventElapsedTime(
			&ms,
			static_cast<CUevent>(ro_Start.m_EventHandle),
			static_cast<CUevent>(ro_End.m_EventHandle)
		);

		if (result == CUDA_SUCCESS) return ms;

		CUDA_ERROR_TRAP(result);
		return 0.0f;
	}

	GpuIpcEventHandle DeviceEvents::getIpcHandle(const GpuEvent& ro_Event) {
		CORIUM_ASSERT(ro_Event.isValid());

		CUipcEventHandle handle;
		const CUresult result = cuIpcGetEventHandle(&handle, static_cast<CUevent>(ro_Event.m_EventHandle));

		if (result == CUDA_SUCCESS) {
			GpuIpcEventHandle ro_Handle;
			std::memcpy(ro_Handle.m_Reserved, &handle, sizeof(CUipcEventHandle));
			return ro_Handle;
		}

		CUDA_ERROR_TRAP(result);
		return GpuIpcEventHandle{};
	}

	GpuEvent DeviceEvents::openIpcHandle(const GpuIpcEventHandle& ro_Handle) {
		CUipcEventHandle handle;
		std::memcpy(&handle, ro_Handle.m_Reserved, sizeof(CUipcEventHandle));

		CUevent hEvent;
		const CUresult result = cuIpcOpenEventHandle(&hEvent, handle);

		if (result == CUDA_SUCCESS) {
			GpuEvent ro_Event;
			ro_Event.m_EventHandle = hEvent;
			return ro_Event;
		}

		CUDA_ERROR_TRAP(result);
		return GpuEvent{};
	}
}
