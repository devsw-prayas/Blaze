#pragma once
#include "Blaze.h"
#include "BlazeEvent.h"

namespace Blaze::TaskEngine {
	template<typename D, typename E = std::conditional_t<ENABLE_EVENT_EMITTERS_BLAZE, Events::EventEmitterPack<>, void>>
	class BLAZE ATaskEngine {
		
	};
}
