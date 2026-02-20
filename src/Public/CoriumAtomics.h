#pragma once
#include "PlatIntrin.h"

namespace Corium::Core::Atomics {
	enum class CORIUM MemoryOrder : std::uint8_t {
		RELAXED = CORIUM_MEMORY_ORDER_RELAXED,
		CONSUME = CORIUM_MEMORY_ORDER_CONSUME,
		ACQUIRE = CORIUM_MEMORY_ORDER_ACQUIRE,
		RELEASE = CORIUM_MEMORY_ORDER_RELEASE,
		ACQ_REL = CORIUM_MEMORY_ORDER_ACQ_REL,
		SEQ_CST = CORIUM_MEMORY_ORDER_SEQ_CST
	};

	template<typename T, typename Valid = Intrinsic::ValidAtomicParameter<T>>
	CORIUM_FORCEINLINE CORIUM_NODISCARD_MSG("Cannot discard an atonic load")
	Valid atomicLoad(Valid* p_Memory, MemoryOrder v_Ordering) {
		switch (v_Ordering)
		{
		case MemoryOrder::RELAXED:
			return AtomicLoad_Relaxed(p_Memory);
		case MemoryOrder::CONSUME:
			return AtomicLoad_Consume(p_Memory);
		case MemoryOrder::ACQUIRE:
			return AtomicLoad_Acquire(p_Memory);
		case MemoryOrder::SEQ_CST:
			return AtomicLoad_SeqCst(p_Memory);
		case MemoryOrder::RELEASE: CORIUM_FALLTHROUGH;
		case MemoryOrder::ACQ_REL: return AtomicLoad_SeqCst(p_Memory);
		}
		CORIUM_UNREACHABLE();
	}
}
