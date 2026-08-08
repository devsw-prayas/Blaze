#include "Corium.h"

#include "CoriumFactory.h"

#include "CoriumThread.h"
#include "CoriumEnvironment.h"

namespace Corium::Core::Factory {
	AThreadFactory::AThreadFactory() {
		init(this->m_LaunchDesc);
		init(this->m_AttrDec);
	}

	void AThreadFactory::buildAttrDesc(AffinityMask v_Mask, ProcessorIdx v_Idx, ThreadPriority v_Priority, uint32_t v_NumaNode, Flag v_IsDetached, Flag v_SupportsIdealProcessor, Flag v_IsGuardsEnabled, ThreadAttrDesc& ro_Desc) {
		setAffinity(ro_Desc, v_Mask);
		setIdealProcessor(ro_Desc, v_Idx);
		shouldSupportIdealProcessor(ro_Desc, v_SupportsIdealProcessor);
		vaGuardEnabled(ro_Desc, v_IsGuardsEnabled);
		setPriority(ro_Desc, v_Priority);
		setNumaNode(ro_Desc, v_NumaNode);
		canDetach(ro_Desc, v_IsDetached);
	}

	void AThreadFactory::buildLaunchDesc(size_t v_VaSize, Flag v_IsSuspended, ThreadLaunchDesc& ro_Desc) {
		setVaSize(ro_Desc, v_VaSize);
		isPreSuspended(ro_Desc, v_IsSuspended);
	}

	ThreadHandle DefaultThreadFactory::createAndStart(Closure<void()> r_Closure, const char* p_ThreadName) noexcept {
		using namespace Memory::Literals;

		m_LaunchDesc.m_State = DescriptorState::UNINITIALIZED;
		init(m_LaunchDesc);
		m_AttrDec.m_State = DescriptorState::UNINITIALIZED;
		init(m_AttrDec);

		buildLaunchDesc(CORIUM_SPACE_TLS_MIN_SIZE * 1_MiB, false, m_LaunchDesc);
		setName(m_LaunchDesc, p_ThreadName);
		attachLaunchAddr(m_LaunchDesc, std::move(r_Closure));
		validate(m_LaunchDesc);

		buildAttrDesc(0, 0, ThreadPriority::NORMAL, 0, false, false, true, this->m_AttrDec);
		validate(m_AttrDec);

		return NativeThread::createThread(std::move(m_LaunchDesc), m_AttrDec);
	}

	AffinityFactory::AffinityFactory(uint32_t v_Node, AffinityMask v_Mask)
		: m_Node(v_Node), m_Mask(v_Mask) {
		const auto& cpuInfo = Environment::EnvironmentProbe::getCpuInfo();
		CORIUM_DEBUG_ASSERT(v_Node < cpuInfo.m_NumaNodeCount &&
			"AffinityFactory node must be a valid, enumerated NUMA node");
		CORIUM_DEBUG_ASSERT((v_Mask & cpuInfo.m_NumaNodeMasks[v_Node]) == v_Mask &&
			"AffinityFactory mask must be a subset of the NUMA node's real processor mask");
	}

	ThreadHandle AffinityFactory::createAndStart(Closure<void()> r_Closure, const char* p_ThreadName) noexcept {
		using namespace Memory::Literals;

		m_LaunchDesc.m_State = DescriptorState::UNINITIALIZED;
		init(m_LaunchDesc);
		m_AttrDec.m_State = DescriptorState::UNINITIALIZED;
		init(m_AttrDec);

		buildLaunchDesc(CORIUM_SPACE_TLS_MIN_SIZE * 1_MiB, false, m_LaunchDesc);
		setName(m_LaunchDesc, p_ThreadName);
		attachLaunchAddr(m_LaunchDesc, std::move(r_Closure));
		validate(m_LaunchDesc);

		buildAttrDesc(m_Mask, 0, ThreadPriority::NORMAL, m_Node, false, false, true, m_AttrDec);
		validate(m_AttrDec);

		return NativeThread::createThread(std::move(m_LaunchDesc), m_AttrDec);
	}

	PriorityFactory::PriorityFactory(uint32_t v_Node, AffinityMask v_Mask, ThreadPriority v_Priority)
		: AffinityFactory(v_Node, v_Mask), m_Priority(v_Priority) {}

	ThreadHandle PriorityFactory::createAndStart(Closure<void()> r_Closure, const char* p_ThreadName) noexcept {
		using namespace Memory::Literals;

		m_LaunchDesc.m_State = DescriptorState::UNINITIALIZED;
		init(m_LaunchDesc);
		m_AttrDec.m_State = DescriptorState::UNINITIALIZED;
		init(m_AttrDec);

		buildLaunchDesc(CORIUM_SPACE_TLS_MIN_SIZE * 1_MiB, false, m_LaunchDesc);
		setName(m_LaunchDesc, p_ThreadName);
		attachLaunchAddr(m_LaunchDesc, std::move(r_Closure));
		validate(m_LaunchDesc);

		buildAttrDesc(m_Mask, 0, m_Priority, m_Node, false, false, true, m_AttrDec);
		validate(m_AttrDec);

		return NativeThread::createThread(std::move(m_LaunchDesc), m_AttrDec);
	}
}
