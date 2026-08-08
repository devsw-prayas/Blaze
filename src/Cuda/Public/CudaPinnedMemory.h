#pragma once
#include "Corium.h"
#include "CoriumCompiler.h"
#include "CoriumDiagnostics.h"
#include "CudaUtils.h"
#include "CudaStream.h"

namespace Corium::Cuda::Memory {	  
	using namespace Utils;
	class CORIUM_RUNTIME_API PinnedMemory final {
	public:
		static PinnedAddress pinnedAlloc(size_t v_Bytes);
		static PinnedAddress pinnedAlloc(size_t v_Bytes, uint32_t v_Flags);
		static void pinnedFree(PinnedAddress& ro_Addr);
		static void mapToDevice(GpuAddress& ro_DevAddr, const PinnedAddress& ro_Addr, uint32_t v_Flags);
		static void hostMemRegister(const PinnedAddress& ro_Addr, size_t v_Bytes, uint32_t v_Flags);
		static void hostMemUnRegister(const PinnedAddress& ro_Addr);

		static void copyHostToDevAsync(
			const GpuAddress& ro_Dst, const PinnedAddress& ro_Src, 
			size_t v_Bytes, const Streams::GpuStream& ro_Stream);

		static void copyDevToHostAsync(
			const PinnedAddress& ro_Dst, const GpuAddress& ro_Src,
			size_t v_Bytes, const Streams::GpuStream& ro_Stream
		);
	};									
}
