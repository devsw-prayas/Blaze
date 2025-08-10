/*
* Copyright (c) 2025 StormWeaver
*
* This file is part of the Blaze Multithreading API
*
* Licensed under the MIT License. You may obtain a copy of the License at
* https://opensource.org/licenses/MIT
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND...
*/

#pragma once
#include "Blaze.h"

namespace Blaze::Events {
	constexpr size_t DEFAULT_HASH = 0xF00D; // From a food lover!

	template<typename T,typename = void>
	struct HasContract : std::false_type {};

	template<typename T>
	struct HasContract<T, std::void_t<typename T::Contract>> : std::true_type {};

	template<typename ContractTag, size_t Hash = DEFAULT_HASH>
	class BLAZE IBlazeEvent {
	public:
		using Contract = ContractTag;
		template<typename Derived, typename...Args>
		static void invoke(Args&&... u_Args) {
			static_assert(std::is_base_of_v<IBlazeEvent, Derived>,
				"Derived must be a subclass of IBlazeEvent");
			Derived::template invoke<Args...>(std::forward<Args>(u_Args)...);
		}
	};

	template<size_t Hash = DEFAULT_HASH ,typename...Args>
	struct BLAZE EventEmitterPack final{
		using EventPack = std::tuple<Args...>;
		constexpr size_t Count = sizeof...(Args);
	private:
		using Indices = std::make_index_sequence<std::tuple_size_v<EventPack>>;

		template<size_t...I>
		static consteval bool iterate(std::index_sequence<I...>) {
			static_assert(std::conjunction_v<HasContract<std::tuple_element_t<I, EventPack>>...>, 
				"All event types must declare a Contract");
			return ((...&& std::is_base_of_v<IBlazeEvent<typename std::tuple_element_t<I, EventPack>::Contract, Hash>, 
				std::tuple_element_t<I, EventPack>>));
		}
	public:
		static_assert(sizeof...(Args) > 0, "EventEmitterPack must contain at least one event types");
		static_assert(iterate(Indices{}), "All events must be derived from IBlazeEvent");
	};
}