#include "Corium.h"
#include "Inductor.h"
#include "CoriumEnvironment.h"
#include "CoriumRuntime.h"

namespace Corium::Execution::Inductor {
    bool TaskInductor::init(TaskDesc& ro_Desc, uint32_t v_Node) {
        if (!CoriumRuntime::isRuntimeInit()) return false;
        if (v_Node >= Environment::EnvironmentProbe::getCpuInfo().m_NumaNodeCount) return false;

        ro_Desc.m_NumaNode = v_Node;
        ro_Desc.m_MemDesc = Memory::UniquePtr<TaskMemoryDesc, Memory::Allocators::TaskMetadataAllocator>{
            Memory::Internal::AtomicAllocators::instance().s_TaskMemoryDescAllocator[v_Node].load()
        };
        ro_Desc.m_State = TaskDescState::MUTABLE;
        return true;
    }
}