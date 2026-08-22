#include "Corium.h"
#include "Builder.h"
#include <cstring>

namespace Corium::Execution {

	void TaskContext::fail(uint32_t v_Code, const char* p_Message) noexcept {
		auto* error = reinterpret_cast<TaskError*>(m_pOutputBuffer);
		if (error->v_Code != 0) return;

		error->v_Code = v_Code;

		if (p_Message) {
			size_t length = 0;
			while (p_Message[length] && length < 123) ++length;
			std::memcpy(error->v_Message, p_Message, length);
			error->v_Message[length] = '\0';
		}

		auto locked = m_Handle.m_wpState.lock();
		if (locked) {
			locked->m_State.store(
				static_cast<uint32_t>(TaskState::Failed),
				Core::Atomics::MemoryOrder::RELEASE);
		}
	}

} // namespace Corium::Execution

namespace Corium::Execution::Builder {

	void TaskBuilder::destroy(TaskFrame& ro_Frame) noexcept {
		if (ro_Frame.m_pClosure && ro_Frame.m_pfnDestroy)
			ro_Frame.m_pfnDestroy(ro_Frame.m_pClosure);

		ro_Frame.m_pClosure      = nullptr;
		ro_Frame.m_pfnDestroy    = nullptr;
		ro_Frame.m_pfnCpuInvoke  = nullptr;
		ro_Frame.m_pfnSliceInvoke = nullptr;
	}

} // namespace Corium::Execution::Builder
