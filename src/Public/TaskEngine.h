/*
* Copyright (c) 2025 StormWeaver
*
* This file is part of the Corium Multithreading API
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
#include "Corium.h"
#include "CoriumEvent.h"
#include "CoriumTraits.h"

namespace Corium::TaskEngine {
	struct EventContractOnStaged final {};
	struct EventContractOnAllStaged final {};
	struct EventContractOnLaunch final {};

#if ENABLE_EVENT_EMITTERS_CORIUM
	template<typename D, typename E = void, size_t Hash = Events::DEFAULT_HASH>
		requires std::disjunction_v<Events::HasContract<E>, std::is_void<E>>
#else
	template<typename D, size_t Hash = Events::DEFAULT_HASH>
#endif
	class CORIUM ATaskEngine {
		static_assert(std::is_final_v<D>, "Concrete implementations must be final");
		using Derived = D;
#if ENABLE_EVENT_EMITTERS_CORIUM
		using EventContract = std::conditional_t<std::is_void_v<E>, E, void>;
		static_assert(!std::is_void_v<EventContract>&& EventContract::Count >= 3,
			"At least 3 Event type hooks must be provided in the EventContract");
		using StagingEvent = std::tuple_element_t<0, typename EventContract::Contract>;
		using StagingAllEvent = std::tuple_element_t<1, typename EventContract::Contract>;
		using LaunchEvent = std::tuple_element_t<2	, typename EventContract::Contract>;
	public:
		using OnStaged = Events::ICoriumEvent<EventContractOnStaged, Hash>;
		using OnAllStaged = Events::ICoriumEvent<EventContractOnAllStaged, Hash>;
		using OnLaunch = Events::ICoriumEvent<EventContractOnLaunch, Hash>;
#endif
	private:

		using Duration = std::chrono::steady_clock::duration;
		using TimePoint = std::chrono::steady_clock::time_point;

		bool m_isRunning;
		bool m_shutdown;

	public:
		ATaskEngine() = default;
		virtual ~ATaskEngine() = default;

		ATaskEngine(const ATaskEngine&) = delete;
		ATaskEngine& operator=(ATaskEngine&) = delete;

		ATaskEngine(ATaskEngine&&) noexcept = delete;
		ATaskEngine&& operator=(ATaskEngine&&) noexcept = delete;

		template<typename F>
		void stage(F&& u_Func) {
			if constexpr (!std::is_void_v<EventContract>) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
				static_assert(std::is_base_of_v<StagingEvent, OnStaged>,
					"The first Event type must be OnStaged hook");
				Events::ICoriumEvent<EventContractOnStaged, Hash>::template invoke<StagingEvent>();
			}
#endif
			static_cast<Derived*>(this)->template stageC<F>(std::forward<F>(u_Func));
		}

		template<typename...Args>
		void stageAll(Args&&...u_Func) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v<StagingAllEvent, OnAllStaged>,
					"The second Event type must be OnAllStaged hook");
				Events::ICoriumEvent<EventContractOnAllStaged, Hash>::template invoke<StagingAllEvent, size_t>(sizeof...(Args));
			}
#endif
			static_cast<Derived*>(this)-> template stageAllC<Args...>(std::forward<Args>(u_Func)...);
		}

		void launch() {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v<LaunchEvent, OnLaunch>,
					"The third Event type must be OnLaunch hook");
				Events::ICoriumEvent<EventContractOnLaunch, Hash>:: template invoke<LaunchEvent>();
			}
#endif
			static_cast<Derived*>(this)->launchC();
		}

		[[nodiscard]] virtual bool isRunning() {
			return this->m_isRunning;
		}
		[[nodiscard]] virtual bool isShutdown() {
			return this->m_shutdown;
		}
		virtual void shutdown() = 0;
		virtual void shutdownNow() = 0;

		template<typename T> requires Traits::IsDurationV<T>
		bool awaitTermination(T&& u_Duration) {
			return static_cast<Derived*>(this)->template awaitTerminationC<T>(std::forward<T>(u_Duration));
		}

		virtual bool cancelAllPending() = 0;
		[[nodiscard]] virtual size_t getActiveTasksCount() = 0;
		[[nodiscard]] virtual size_t getCompletedTasksCount() = 0;

		[[nodiscard]] virtual size_t getTasksCompletedInCore(size_t v_CoreIdx) = 0;
		[[nodiscard]] virtual size_t getTasksPendingInCore(size_t v_CoreIdx) = 0;
	};
}
