#pragma once
#include <Corium.h>

#include "CoriumMemory.h"

namespace Corium::Builder {
	enum class ParamMemState : uint8_t {
		UNINITIALIZED, UPDATING, FROZEN
	};

	struct CORIUM TaskMemoryDesc final {
		TaskMemoryDesc() :
			m_InputMemBuffer(nullptr), m_OutputMemBuffer(nullptr),
			m_InputBufferSize(0), m_OutputBufferSize(0),
			m_MaxInputsSizeBuffer(nullptr), m_MaxInputsAlignmentBuffer(nullptr),
			m_MaxOutputsSizeBuffer(nullptr), m_MaxOutputsAlignmentBuffer(nullptr),
			m_MaxInputs(0), m_MaxOutputs(0), m_MemState(ParamMemState::UNINITIALIZED) {
		}

		~TaskMemoryDesc() = default;

		TaskMemoryDesc(const TaskMemoryDesc&) = delete;
		TaskMemoryDesc& operator=(const TaskMemoryDesc&) = delete;

		TaskMemoryDesc(TaskMemoryDesc&&) noexcept = default;
		TaskMemoryDesc& operator=(TaskMemoryDesc&&) noexcept = default;
	private:
		size_t m_InputBufferSize;
		size_t m_OutputBufferSize;

		Memory::UniquePtr<void> m_MaxInputsSizeBuffer;
		Memory::UniquePtr<void> m_MaxInputsAlignmentBuffer;

		Memory::UniquePtr<void> m_MaxOutputsSizeBuffer;
		Memory::UniquePtr<void> m_MaxOutputsAlignmentBuffer;

		size_t m_MaxInputs;
		size_t m_MaxOutputs;

		ParamMemState m_MemState;
	};

	struct TaskMemory final {
	private:
		Memory::UniquePtr<void> m_InputMemBuffer;
		Memory::UniquePtr<void> m_OutputMemBuffer;
	};

	struct CORIUM TaskFrame final {

	private:
		const Memory::WeakPtr<TaskMemoryDesc> m_MemDesc;
		friend struct TaskMemoryDesc;
	};

	class CORIUM TaskBuilder final {

		friend struct TaskMemoryDesc;
	};
}
