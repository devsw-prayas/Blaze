#include "Corium.h"
#include "Builder.h"
#include <cstring>

namespace Corium::Execution {

	void TaskContext::fail(uint32_t v_Code, const char* p_Message) noexcept {
		auto* p_Err = reinterpret_cast<TaskError*>(m_pOutputBuffer);
		if (p_Err->v_Code != 0) return;

		p_Err->v_Code = v_Code;

		if (p_Message) {
			size_t v_Len = 0;
			while (p_Message[v_Len] && v_Len < 123) ++v_Len;
			std::memcpy(p_Err->v_Message, p_Message, v_Len);
			p_Err->v_Message[v_Len] = '\0';
		}

		auto v_Locked = m_Handle.m_wpState.lock();
		if (v_Locked) {
			v_Locked->m_State.store(
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
