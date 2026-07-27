#include "Corium.h"
#include "Inductor.h"
#include "PipelineUtils.h"
#include "CoriumEnvironment.h"
#include "CoriumRuntime.h"

#include <cstring>

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

    TaskFrame TaskInductor::cook(TaskDesc& ro_Desc) {
        if (!validate(ro_Desc)) return TaskFrame{};

        const uint32_t v_Node   = ro_Desc.m_NumaNode;
        const uint32_t v_InSz   = ro_Desc.m_MemDesc->m_InputBufferSize;
        const uint32_t v_OutSz  = ro_Desc.m_MemDesc->m_OutputBufferSize;

        auto& r_Allocs   = Memory::Internal::AtomicAllocators::instance();
        auto* p_PayAlloc = r_Allocs.s_TaskPayloadAllocator[v_Node].load();

        void* p_In  = p_PayAlloc->allocateImpl(v_InSz,  32);
        void* p_Out = p_PayAlloc->allocateImpl(v_OutSz, 32);
        if (!p_In || !p_Out) return TaskFrame{};

        std::memset(p_Out, 0, sizeof(Corium::Execution::TaskError));

        ro_Desc.m_State = TaskDescState::FROZEN;

        Memory::WeakPtr<TaskMemoryDesc, Memory::Allocators::TaskMetadataAllocator> v_Wp{ ro_Desc.m_MemDesc };

        Memory::Internal::VARegion v_InBuf { static_cast<uint8_t*>(p_In),  v_InSz  };
        Memory::Internal::VARegion v_OutBuf{ static_cast<uint8_t*>(p_Out), v_OutSz };

        return TaskFrame{ std::move(v_Wp), v_InBuf, v_OutBuf };
    }

    bool TaskInductor::reset(TaskDesc& ro_Desc) {
        ro_Desc.m_MemDesc.reset();
        ro_Desc.m_State    = TaskDescState::UNINITIALIZED;
        ro_Desc.m_NumaNode = 0;
        return true;
    }
}
