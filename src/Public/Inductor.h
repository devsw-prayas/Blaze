#pragma once
#include <Corium.h>

#include "Builder.h"
#include "CoriumTraits.h"

namespace Corium::Execution::Inductor {

	using TaskBitFlag = size_t;

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
		TaskDescState m_State;
		TaskBitFlag m_Flag;
	};


	class CORIUM TaskInductor final {
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

		friend struct TaskDesc;
	};
}
