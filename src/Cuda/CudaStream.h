#pragma once
#include "Corium.h"
#include "CoriumCompiler.h"

namespace Corium::Cuda::Streams {
	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(8) GpuStream final {
		void* m_StreamHandle = nullptr;

		GpuStream() = default;
		~GpuStream() = default;

		GpuStream(const GpuStream&) = default;
		GpuStream& operator=(const GpuStream&) = default;

		GpuStream(GpuStream&&) noexcept = default;
		GpuStream& operator=(GpuStream&&) noexcept = default;

		CORIUM_NODISCARD bool isValid() const {
			return m_StreamHandle != nullptr;
		}
	};
}
