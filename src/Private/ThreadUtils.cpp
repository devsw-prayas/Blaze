#include <Corium.h>
#include <ThreadUtils.h>

namespace Corium::Core {
	void init(ThreadAttrDesc& ro_Desc) {
		if (ro_Desc.m_State == DescriptorState::FROZEN) return;
		ro_Desc.m_State = DescriptorState::UNINITIALIZED;
		ro_Desc.m_IdealProcessor = 0;
		ro_Desc.m_IsGuardPageEnabled = false;
		ro_Desc.m_Mask = 0;
		ro_Desc.m_SupportsIdealProcessor = false;
		ro_Desc.m_ThreadPriority = 0;
		ro_Desc.m_isDetached = false;
	}

	void setAffinity(ThreadAttrDesc& ro_Desc, AffinityMask v_Mask) {
		if (isFrozen(ro_Desc)) return;
		promoteMutable(ro_Desc);
		ro_Desc.m_Mask = v_Mask;
	}

	void setIdealProcessor(ThreadAttrDesc& ro_Desc, ProcessorIdx v_Idx) {
		if (isFrozen(ro_Desc)) return;
		promoteMutable(ro_Desc);
		ro_Desc.m_IdealProcessor = v_Idx;
	}

	void shouldSupportIdealProcessor(ThreadAttrDesc& ro_Desc, bool v_Permission) {
		if (isFrozen(ro_Desc)) return;
		promoteMutable(ro_Desc);
		ro_Desc.m_SupportsIdealProcessor = v_Permission;
	}

	void canDetach(ThreadAttrDesc& ro_Desc, bool v_Permission) {
		if (isFrozen(ro_Desc)) return;
		promoteMutable(ro_Desc);
		ro_Desc.m_isDetached = v_Permission;
	}

	void vaGuardEnabled(ThreadAttrDesc& ro_Desc, bool v_Permission) {
		if (isFrozen(ro_Desc)) return;
		promoteMutable(ro_Desc);
		ro_Desc.m_IsGuardPageEnabled = v_Permission;
	}

	bool validate(ThreadAttrDesc& ro_Desc) {
		if (isFrozen(ro_Desc)) return false;
		if (ro_Desc.m_State == DescriptorState::UNINITIALIZED) return false;
		if (!ro_Desc.m_SupportsIdealProcessor && ro_Desc.m_IdealProcessor > 0) return false;
		ro_Desc.m_State = DescriptorState::FROZEN;
		return true;
	}









} // namespace Corium::Core