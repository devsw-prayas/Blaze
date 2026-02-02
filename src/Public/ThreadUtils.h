#pragma once
#include <Corium.h>

namespace Corium::Core {
	using AffinityMask = size_t;
	using ProcessorIdx = uint32_t;
	using ThreadPriority = int32_t;		  

	enum class DescriptorState :uint8_t {
		UNINITIALIZED, MUTABLE, FROZEN
	};

	struct CORIUM alignas(64) ThreadAttrDesc final {
		AffinityMask m_Mask;
		ProcessorIdx m_IdealProcessor;
		ThreadPriority m_ThreadPriority;
		DescriptorState m_State = DescriptorState::UNINITIALIZED;
		bool m_isDetached;
		bool m_SupportsIdealProcessor;
		bool m_IsGuardPageEnabled;

		ThreadAttrDesc() = default;
		~ThreadAttrDesc() = default;

		ThreadAttrDesc(const ThreadAttrDesc&) = default;
		ThreadAttrDesc& operator=(const ThreadAttrDesc&) = default;
		ThreadAttrDesc(ThreadAttrDesc&&) noexcept = default;
		ThreadAttrDesc& operator=(ThreadAttrDesc&&) noexcept = default;
	};

	void CORIUM init(ThreadAttrDesc& ro_Desc);
	void CORIUM setAffinity(ThreadAttrDesc& ro_Desc, AffinityMask v_Mask);
	void CORIUM shouldSupportIdealProcessor(ThreadAttrDesc& ro_Desc, bool v_Permission);
	void CORIUM setIdealProcessor(ThreadAttrDesc& ro_Desc, ProcessorIdx v_Idx);
	void CORIUM canDetach(ThreadAttrDesc& ro_Desc, bool v_Permission);
	void CORIUM vaGuardEnabled(ThreadAttrDesc& ro_Desc, bool v_Permission);


}