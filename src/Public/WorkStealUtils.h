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
#include "BlazeEvent.h"
#include "BlazeMemory.h"
#include "BlazeTraits.h"
#include "BlazeUtils.h"

namespace Blaze::WorkSteal {
	struct EventContractOnFork final {};
	struct EventContractOnJoin final {};
	struct EventContractOnCompute final {};

#if ENABLE_EVENT_EMITTERS_BLAZE
	template<typename D, typename E = void, size_t Hash = Events::DEFAULT_HASH>
		requires std::disjunction_v<Events::HasContract<E>, std::is_void<E>>
#else
	template<typename D, size_t Hash = Events::DEFAULT_HASH>
#endif
	class BLAZE IWorkStealLoad {
		using Derived = D;

		static_assert(std::is_final_v<D>, "Concrete implementations must be final");

#if ENABLE_EVENT_EMITTERS_BLAZE
		using EventContract = E;
		static_assert(!std::is_void_v<EventContract>&& EventContract::Count >= 4,
			"At least 3 event hook types must be provided in the EventContract");

		using OnFork = Events::IBlazeEvent<EventContractOnFork, Hash>;
		using OnCompute = Events::IBlazeEvent<EventContractOnCompute, Hash>;
		using OnJoin = Events::IBlazeEvent<EventContractOnJoin, Hash>;

		using ForkEvent = std::tuple_element_t<0, typename EventContract::Contract>;
		using JoinEvent = std::tuple_element_t<1, typename EventContract::Contract>;
		using ComputeEvent = std::tuple_element_t<2, typename EventContract::Contract>;
	public:
		struct DefaultOnForkContract final : OnFork {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnJoinContract final : OnJoin {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnComputeContract final : OnCompute {
			static void invoke() {/* Default implementation does nothing */ }
		};
#endif

	protected:
		void* m_context = nullptr;
		Memory::SharedPointer<Utils::IHandle>(*m_fork)(Derived&&) = nullptr;
		void(*m_join)(Memory::SharedPointer<Utils::IHandle>) = nullptr;

	public:
		decltype(auto) compute() requires(Traits::hasComputeC<Derived>) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnCompute, ComputeEvent>,
					"The third Event type must be a OnCompute Hook");
				Events::IBlazeEvent<EventContractOnCompute, Hash>::template invoke<ComputeEvent>();
			}

			return static_cast<Derived*>(this)->computeC();
		}

		Memory::SharedPointer<Utils::IHandle> fork(Derived&& u_subTask) requires(Traits::hasForkC<Derived>) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnFork, ForkEvent>,
					"The first Event type must be a OnFork Hook");
				Events::IBlazeEvent<EventContractOnFork, Hash>::template invoke<ForkEvent>();
			}

			return static_cast<Derived*>(this)->forkC(std::move(u_subTask));
		}

		void join(Memory::SharedPointer<Utils::IHandle> handle) requires Traits::hasJoinC<Derived> {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnJoin, JoinEvent>,
					"The second Event type must be a OnJoin Hook");
				Events::IBlazeEvent<EventContractOnJoin, Hash>::template invoke<JoinEvent>();
			}
			static_cast<Derived*>(this)->join(handle);
		}

		[[nodiscard]] bool canJoin() {
			return static_cast<Derived*>(this)->canJoin();
		}
	};
}
