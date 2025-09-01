#pragma once

#include "CoriumEvent.h"
#include "Executors.h"


namespace Corium::Composable {

	enum class Compose : uint8_t {
		TRANSFORM, INTERMEDIATE, REDUCTION
	};

	struct ComposableHandle : Utils::IHandle {
		//TODO STUB
	};

	struct Composable {
		//TODO intrusive list
	};

#if ENABLE_EVENT_EMITTERS_CORIUM
	template<typename A, typename E = void, size_t Hash = Corium::Events::DEFAULT_HASH>
		requires std::disjunction_v<Corium::Events::HasContract<E>, std::is_void<E>>
#else
	template<typename A>
#endif
	class IComposer : Executors::IExecutorVirtual {
	public:
		
	};

}
