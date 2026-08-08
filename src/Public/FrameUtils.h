#pragma once
#include "CoriumCompiler.h"
#include "CoriumMemory.h"
#include "CoriumUtility.h"

namespace Corium::Core::Frame {
	using namespace Corium::Memory::Literals;

	inline constexpr size_t kReservedHeaderSize = 320;
	inline constexpr size_t kFrameStackSize = (CORIUM_DEFAULT_FRAME_STACK_SIZE) * 1_MiB;
	inline constexpr size_t kFrameMaxStackSize = 8_MiB;

	CORIUM_STATIC_ASSERT(kFrameMaxStackSize > kFrameStackSize, "Invalid Default Frame Stack size. Stack"
		"size must be less than 8MiB");

	enum class CORIUM_RUNTIME_API FrameState : uint8_t {
		READY, SUSPENDED, TERMINATED
	};

	enum class CORIUM_RUNTIME_API Provenance : uint8_t {
		NUMA_GLOBAL,
		CALLEE_OWNED
	};

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(16) FrameHandle final {
		Utils::FunctionView<void(void*)> m_Entry;
		void* m_StackPtr;
		size_t m_StackSize;
		void* m_RegBlock;
		uint8_t m_Origin;
		uint8_t m_NumaNode;
		FrameState m_State;
		uint8_t m_Reserved[16];
	};
}
