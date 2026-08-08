#pragma once
#include "Corium.h"
#include "CoriumCompiler.h"
#include "CudaUtils.h"

namespace Corium::Cuda::Streams {
	using namespace Utils;

	class CORIUM_RUNTIME_API DeviceStreams final {
	public:
		// Core Stream Management
		static GpuStream createStream(StreamFlags v_Flags);
		static GpuStream createStreamWithPriority(StreamFlags v_Flags, int v_Priority);
		static void      destroyStream(GpuStream& ro_Stream);
		static void      syncStream(const GpuStream& ro_Stream);
		static bool      queryStream(const GpuStream& ro_Stream);

		// Synchronization
		static void streamWaitEvent(GpuStream& ro_Stream, const GpuEvent& ro_Event);
		static void addStreamCallback(GpuStream& ro_Stream, void* p_Callback, void* p_UserData);

		// Graph Capture
		static void     beginStreamCapture(GpuStream& ro_Stream, StreamCaptureMode v_Mode);
		static GpuGraph endStreamCapture(GpuStream& ro_Stream);
	};
}
