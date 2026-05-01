#include "Corium.h"
#include "CudaStream.h"

#define ALLOW_SYSCALL
#include "CoriumSyscalls.h"

#define ALLOW_HELPERS
#include "CudaInternalHelpers.h"

namespace Corium::Cuda::Streams {
	GpuStream DeviceStreams::createStream(StreamFlags v_Flags) {
		CUstream stream;
		const uint32_t flags = Internal::CUDA_InternalHelpers::toStreamFlags(v_Flags);
		const CUresult result = cuStreamCreate(&stream, flags);

		if (result == CUDA_SUCCESS) {
			GpuStream ro_Stream;
			ro_Stream.m_StreamHandle = stream;
			return ro_Stream;
		}

		CUDA_ERROR_TRAP(result);
		return GpuStream{};
	}

	GpuStream DeviceStreams::createStreamWithPriority(StreamFlags v_Flags, int v_Priority) {
		CUstream stream;
		const uint32_t flags = Internal::CUDA_InternalHelpers::toStreamFlags(v_Flags);
		const CUresult result = cuStreamCreateWithPriority(&stream, flags, v_Priority);

		if (result == CUDA_SUCCESS) {
			GpuStream ro_Stream;
			ro_Stream.m_StreamHandle = stream;
			return ro_Stream;
		}

		CUDA_ERROR_TRAP(result);
		return GpuStream{};
	}

	void DeviceStreams::destroyStream(GpuStream& ro_Stream) {
		if (!ro_Stream.isValid()) return;

		const CUresult result = cuStreamDestroy(static_cast<CUstream>(ro_Stream.m_StreamHandle));
		if (result == CUDA_SUCCESS) {
			ro_Stream.m_StreamHandle = nullptr;
			return;
		}

		CUDA_ERROR_TRAP(result);
	}

	void DeviceStreams::syncStream(const GpuStream& ro_Stream) {
		CORIUM_ASSERT(ro_Stream.isValid());

		const CUresult result = cuStreamSynchronize(static_cast<CUstream>(ro_Stream.m_StreamHandle));
		if (result == CUDA_SUCCESS) return;

		CUDA_ERROR_TRAP(result);
	}

	bool DeviceStreams::queryStream(const GpuStream& ro_Stream) {
		if (!ro_Stream.isValid()) return true;

		const CUresult result = cuStreamQuery(static_cast<CUstream>(ro_Stream.m_StreamHandle));
		if (result == CUDA_SUCCESS)          return true;
		if (result == CUDA_ERROR_NOT_READY)  return false;

		CUDA_ERROR_TRAP(result);
		return false;
	}

	void DeviceStreams::streamWaitEvent(GpuStream& ro_Stream, const GpuEvent& ro_Event) {
		CORIUM_ASSERT(ro_Stream.isValid());
		CORIUM_ASSERT(ro_Event.isValid());

		const CUresult result = cuStreamWaitEvent(
			static_cast<CUstream>(ro_Stream.m_StreamHandle),
			static_cast<CUevent>(ro_Event.m_EventHandle),
			0
		);

		if (result == CUDA_SUCCESS) return;

		CUDA_ERROR_TRAP(result);
	}

	void DeviceStreams::addStreamCallback(GpuStream& ro_Stream, void* p_Callback, void* p_UserData) {
		CORIUM_ASSERT(ro_Stream.isValid());
		CORIUM_ASSERT(p_Callback != nullptr);

		const CUresult result = cuStreamAddCallback(
			static_cast<CUstream>(ro_Stream.m_StreamHandle),
			reinterpret_cast<CUstreamCallback>(p_Callback),
			p_UserData,
			0
		);

		if (result == CUDA_SUCCESS) return;

		CUDA_ERROR_TRAP(result);
	}

	void DeviceStreams::beginStreamCapture(GpuStream& ro_Stream, StreamCaptureMode v_Mode) {
		CORIUM_ASSERT(ro_Stream.isValid());

		const CUstreamCaptureMode mode = Internal::CUDA_InternalHelpers::toCudaStreamCaptureMode(v_Mode);
		const CUresult result = cuStreamBeginCapture(static_cast<CUstream>(ro_Stream.m_StreamHandle), mode);

		if (result == CUDA_SUCCESS) return;

		CUDA_ERROR_TRAP(result);
	}

	GpuGraph DeviceStreams::endStreamCapture(GpuStream& ro_Stream) {
		CORIUM_ASSERT(ro_Stream.isValid());

		CUgraph graph;
		const CUresult result = cuStreamEndCapture(static_cast<CUstream>(ro_Stream.m_StreamHandle), &graph);

		if (result == CUDA_SUCCESS) {
			GpuGraph ro_Graph;
			ro_Graph.m_GraphHandle = graph;
			return ro_Graph;
		}

		CUDA_ERROR_TRAP(result);
		return GpuGraph{};
	}
}
