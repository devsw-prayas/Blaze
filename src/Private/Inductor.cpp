#include "Corium.h"
#include "Inductor.h"
#include "CoriumEnvironment.h"
#include "CoriumRuntime.h"

namespace Corium::Execution::Inductor {

    bool TaskInductor::init(TaskDesc& ro_Desc, uint32_t v_Node) {
        if (!CoriumRuntime::isRuntimeInit()) return false;
        if (v_Node >= Environment::EnvironmentProbe::getCpuInfo().m_NumaNodeCount) return false;

        auto& r_Allocs = Memory::Internal::AtomicAllocators::instance();
        ro_Desc.m_MemDesc = Memory::SharedPtr<TaskMemoryDesc, Memory::Allocators::TaskMetadataAllocator>::make(
            r_Allocs.s_TaskMemoryDescAllocator[v_Node].load(),
            r_Allocs.s_ControlBlockAllocator[v_Node].load()
        );
        if (!ro_Desc.m_MemDesc) return false;

        ro_Desc.m_NumaNode = v_Node;
        ro_Desc.m_State    = TaskDescState::MUTABLE;
        return true;
    }

    bool TaskInductor::validate(const TaskDesc& ro_Desc) {
        if (ro_Desc.m_State != TaskDescState::MUTABLE) return false;
        if (!ro_Desc.m_MemDesc) return false;
        if (ro_Desc.m_MemDesc->m_MemState != ParamMemState::FROZEN) return false;
        if (!ro_Desc.m_MemDesc->m_InputMPH.isValid()) return false;
        if (!ro_Desc.m_MemDesc->m_OutputMPH.isValid()) return false;
        return true;
    }

    bool TaskInductor::cook(TaskDesc& ro_Desc) {
        if (!validate(ro_Desc)) return false;
        ro_Desc.m_State = TaskDescState::FROZEN;
        return true;
    }

    bool TaskInductor::reset(TaskDesc& ro_Desc) {
        ro_Desc.m_MemDesc.reset();
        ro_Desc.m_State    = TaskDescState::UNINITIALIZED;
        ro_Desc.m_NumaNode = 0;
        ro_Desc.m_Flag     = 0;
        return true;
    }
}
