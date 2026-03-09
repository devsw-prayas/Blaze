#pragma once
#include <Corium.h>

#include "CoriumTraits.h"
#include <CoriumMemory.h>

namespace Corium::Execution::Inductor {

	using TaskBitFlag = size_t;

	enum class ParamMemState : uint8_t {
		UNINITIALIZED, UPDATING, FROZEN
	};

	struct CORIUM_RUNTIME_API alignas(64) TaskMemoryDesc final {
		TaskMemoryDesc() :
			m_InputBufferSize(0), m_OutputBufferSize(0),
			m_MaxInputs(0), m_MaxOutputs(0), m_MemState(ParamMemState::UNINITIALIZED) {
		}

		~TaskMemoryDesc() = default;

		TaskMemoryDesc(const TaskMemoryDesc&) = delete;
		TaskMemoryDesc& operator=(const TaskMemoryDesc&) = delete;

		TaskMemoryDesc(TaskMemoryDesc&&) noexcept = default;
		TaskMemoryDesc& operator=(TaskMemoryDesc&&) noexcept = default;
	private:
		uint32_t m_InputBufferSize;
		uint32_t m_OutputBufferSize;

		Memory::UniquePtr<void> m_MaxInputsSizeBuffer;
		Memory::UniquePtr<void> m_MaxInputsAlignmentBuffer;

		Memory::UniquePtr<void> m_MaxOutputsSizeBuffer;
		Memory::UniquePtr<void> m_MaxOutputsAlignmentBuffer;

		uint32_t m_MaxInputs;
		uint32_t m_MaxOutputs;

		ParamMemState m_MemState;
	};

	enum class TaskDescState : uint8_t {
		UNINITIALIZED, MUTABLE, FROZEN
	};

	struct TaskDesc final {
		TaskDesc() : m_State(TaskDescState::UNINITIALIZED), m_Flag(0) {}
		~TaskDesc() = default;

		TaskDesc(const TaskDesc&) = default;
		TaskDesc& operator=(const TaskDesc&) = default;

		TaskDesc(TaskDesc&&) noexcept = default;
		TaskDesc& operator=(TaskDesc&&) noexcept = default;
	private:
		Memory::UniquePtr<TaskMemoryDesc> m_MemDesc;
		TaskDescState m_State;
		TaskBitFlag m_Flag;

		friend struct TaskMemoryDesc;

	};

	class CORIUM_RUNTIME_API TaskInductor final {
		static bool init(const TaskDesc& ro_Desc);

		template<typename Mutator, bool Permissions>
		requires Backend::Traits::IsMutatorConditional<Mutator, Permissions>::value
		static bool mutate(const TaskDesc& ro_Desc);

		template<typename...Args>
		static bool parameterize(const TaskDesc& ro_Desc);

		template<typename ...Args>
		static bool induce(const TaskDesc& ro_Desc);

		static bool validate(const TaskDesc& ro_Desc);

		static bool cook(const TaskDesc& ro_Desc);

		static bool reset(const TaskDesc& ro_Desc);

		friend struct TaskDesc;
	};
}
