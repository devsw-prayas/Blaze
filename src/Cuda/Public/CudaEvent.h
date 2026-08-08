#pragma once
#include "Corium.h"
#include "CoriumCompiler.h"
#include "CudaUtils.h"

namespace Corium::Cuda::Events {
	using namespace Utils;

	class CORIUM_RUNTIME_API DeviceEvents final {
	public:
		// Core Management
		static GpuEvent createEvent(uint32_t v_Flags);
		static void     destroyEvent(GpuEvent& ro_Event);
		static void     recordEvent(const GpuEvent& ro_Event, const GpuStream& ro_Stream);
		static void     syncEvent(const GpuEvent& ro_Event);
		static bool     queryEvent(const GpuEvent& ro_Event);

		// Timing
		static float elapsedTime(const GpuEvent& ro_Start, const GpuEvent& ro_End);

		// IPC
		static GpuIpcEventHandle getIpcHandle(const GpuEvent& ro_Event);
		static GpuEvent          openIpcHandle(const GpuIpcEventHandle& ro_Handle);
	};
}
