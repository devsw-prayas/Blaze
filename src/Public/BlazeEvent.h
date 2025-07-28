#pragma once
#include "Blaze.h"

namespace Blaze::Events {
	constexpr size_t DEFAULT_HASH = 0xF00D; // From a food lover!
	class BLAZE IBlazeEvent {
	public:
		template<typename Derived, size_t Hash = DEFAULT_HASH, typename...Args>
		void invoke(Args&&... u_Args) {
			static_cast<Derived*>(this)->template invoke<Args...>(std::forward<Args>(u_Args)...);
		}
	};

	template<typename...Args>
	struct BLAZE EventEmitterPack final{
		static_assert((std::is_base_of_v<IBlazeEvent, Args> &&...), "All events must be derived from IBlazeEvent");
		using EventPack = std::tuple<Args...>;
	};
}