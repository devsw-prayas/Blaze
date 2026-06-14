/*
* Copyright (c) 2025 StormWeaver
*
* This file is part of the Corium Multithreading API
*
* Licensed under the MIT License. You may obtain a copy of the License at
* https://opensource.org/licenses/MIT
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND...
*/
#pragma once
#include <Corium.h>

#include "IrUtils.h"
#include "CoriumTraits.h"
#include <CoriumMemory.h>
#include "CoriumPointers.h"
#include "CoriumTraitsSemantics.h"

namespace Corium::Execution::Inductor {
	namespace IR = Corium::IntermediateRepresentation;
	using TaskBitFlag = size_t;

	enum class ParamMemState : uint8_t {
		UNINITIALIZED, UPDATING, FROZEN
	};

	enum class TaskDescState : uint8_t {
		UNINITIALIZED, MUTABLE, FROZEN
	};

	struct CORIUM_RUNTIME_API TaskMemoryDesc final {
		TaskMemoryDesc() = default;
		~TaskMemoryDesc() = default;

		TaskMemoryDesc(const TaskMemoryDesc&) = delete;
		TaskMemoryDesc& operator=(const TaskMemoryDesc&) = delete;

		TaskMemoryDesc(TaskMemoryDesc&&) noexcept = default;
		TaskMemoryDesc& operator=(TaskMemoryDesc&&) noexcept = default;

		IR::MphTable m_InputMPH;
		IR::MphTable m_OutputMPH;

		uint32_t      m_InputBufferSize  = 0;
		uint32_t      m_OutputBufferSize = 0;
		uint32_t      m_MaxInputs        = 0;
		uint32_t      m_MaxOutputs       = 0;
		ParamMemState m_MemState         = ParamMemState::UNINITIALIZED;

	private:
		friend class TaskInductor;
	};

	struct CORIUM_RUNTIME_API CORIUM_ALIGNAS(32) TaskDesc final {
		TaskDesc() : m_MemDesc(), m_NumaNode(0), m_State(TaskDescState::UNINITIALIZED), m_Flag(0) {}
		~TaskDesc() = default;

		TaskDesc(const TaskDesc&) = delete;
		TaskDesc& operator=(const TaskDesc&) = delete;

		TaskDesc(TaskDesc&&) noexcept = default;
		TaskDesc& operator=(TaskDesc&&) noexcept = default;

	private:
		Memory::SharedPtr<TaskMemoryDesc, Memory::Allocators::TaskMetadataAllocator> m_MemDesc;
		uint32_t      m_NumaNode;
		TaskDescState m_State;
		TaskBitFlag   m_Flag;

		friend class TaskInductor;
	};

	class CORIUM_RUNTIME_API TaskInductor final {
	public:
		static bool init(TaskDesc& ro_Desc, uint32_t v_Node);

		template<typename Mutator, bool Permissions>
			requires Backend::Traits::IsMutatorConditional<Mutator, Permissions>::value
		static bool mutate(TaskDesc& ro_Desc) {
			if (ro_Desc.m_State != TaskDescState::MUTABLE) return false;
			constexpr Backend::Traits::TaskBits bit = Backend::Traits::TraitToBit<Mutator>::value;
			CORIUM_STATIC_ASSERT(bit != Backend::Traits::TaskBits::Invalid, "Mutator maps to Invalid bit");
			if (Permissions) ro_Desc.m_Flag |= Backend::Traits::toMask(bit);
			else             ro_Desc.m_Flag &= ~Backend::Traits::toMask(bit);
			return true;
		}

		template<typename InPack>
		static bool parameterize(TaskDesc& ro_Desc) {
			if (ro_Desc.m_State != TaskDescState::MUTABLE) return false;
			if (!ro_Desc.m_MemDesc) return false;
			if (ro_Desc.m_MemDesc->m_MemState != ParamMemState::UNINITIALIZED) return false;

			using Layout = IR::TaskLayout<InPack>;
			static constexpr IR::MphTable s_Mph = Layout::buildMphTable(0);

			ro_Desc.m_MemDesc->m_InputMPH        = s_Mph;
			ro_Desc.m_MemDesc->m_InputBufferSize  = static_cast<uint32_t>(Layout::TotalSize);
			ro_Desc.m_MemDesc->m_MaxInputs        = static_cast<uint32_t>(InPack::Count);
			ro_Desc.m_MemDesc->m_MemState         = ParamMemState::UPDATING;
			return true;
		}

		template<typename OutPack>
		static bool induce(TaskDesc& ro_Desc) {
			if (ro_Desc.m_State != TaskDescState::MUTABLE) return false;
			if (!ro_Desc.m_MemDesc) return false;
			if (ro_Desc.m_MemDesc->m_MemState != ParamMemState::UPDATING) return false;

			using Layout = IR::TaskLayout<OutPack>;
			// Output buffer: [0..127] = TaskError header, [128..] = user params
			static constexpr IR::MphTable s_Mph = Layout::buildMphTable(128);

			ro_Desc.m_MemDesc->m_OutputMPH        = s_Mph;
			ro_Desc.m_MemDesc->m_OutputBufferSize  = static_cast<uint32_t>(Layout::TotalSize + 128);
			ro_Desc.m_MemDesc->m_MaxOutputs        = static_cast<uint32_t>(OutPack::Count);
			ro_Desc.m_MemDesc->m_MemState          = ParamMemState::FROZEN;
			return true;
		}

		static bool validate(const TaskDesc& ro_Desc);
		static bool cook(TaskDesc& ro_Desc);
		static bool reset(TaskDesc& ro_Desc);

		friend struct TaskDesc;
	};
}
