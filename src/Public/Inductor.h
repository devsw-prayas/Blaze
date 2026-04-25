#pragma once
#include <Corium.h>

#include "CoriumTraits.h"
#include <CoriumMemory.h>
#include "CoriumPointers.h"
#include "CoriumTraitsSemantics.h"
#include "CoriumTraitsSemantics.h"

namespace Corium::Execution::Inductor {
	using TaskBitFlag = size_t;

	enum class ParamMemState : uint8_t {
		UNINITIALIZED, UPDATING, FROZEN
	};

	struct CORIUM_RUNTIME_API TaskMemoryDesc final {
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

		uint32_t m_MaxInputs;
		uint32_t m_MaxOutputs;
		ParamMemState m_MemState;
	};

	enum class TaskDescState : uint8_t {
		UNINITIALIZED, MUTABLE, FROZEN
	};

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(32) TaskDesc final {
		TaskDesc() :m_MemDesc(), m_NumaNode(0), m_State(TaskDescState::UNINITIALIZED), m_Flag(0) {}
		~TaskDesc() = default;

		TaskDesc(const TaskDesc&) = delete;
		TaskDesc& operator=(const TaskDesc&) = delete;

		TaskDesc(TaskDesc&&) noexcept = default;
		TaskDesc& operator=(TaskDesc&&) noexcept = default;
	private:
		Memory::UniquePtr<TaskMemoryDesc, Memory::Allocators::TaskMetadataAllocator> m_MemDesc;
		uint32_t m_NumaNode;
		TaskDescState m_State;
		TaskBitFlag m_Flag;

		friend class TaskInductor;
	};

	class CORIUM_RUNTIME_API TaskInductor final {
		static bool init(TaskDesc& ro_Desc, uint32_t v_Node);

		template<typename Mutator, bool Permissions>
			requires Backend::Traits::IsMutatorConditional<Mutator, Permissions>::value
		static bool mutate(TaskDesc& ro_Desc) {
			if (ro_Desc.m_State != TaskDescState::MUTABLE) return false;
			constexpr Backend::Traits::TaskBits bit = Backend::Traits::TraitToBit<Mutator>::value;
			CORIUM_STATIC_ASSERT(bit != Backend::Traits::TaskBits::Invalid, "Mutator maps to Invalid bit");
			if (Permissions) ro_Desc.m_Flag |= Backend::Traits::toMask(bit);
			else ro_Desc.m_Flag &= ~Backend::Traits::toMask(bit);
			return true;
		}

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
