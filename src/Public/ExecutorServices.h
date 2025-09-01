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
#include "Executors.h"
#include "WorkStealUtils.h"

namespace Corium::Executors::Services {
#ifndef NO_OP_DEFAULT_INSTANTANEOUS
#define NO_OP_DEFAULT_INSTANTANEOUS \
	IThreadExecutorService::DefaultOnSubmitContract,\
	IThreadExecutorService::DefaultOnFutureContract,\
	IThreadExecutorService::DefaultOnRunContract,\
	IThreadExecutorService::DefaultCallContract

#endif

#ifndef NO_OP_DEFAULT_SCHEDULED
#define NO_OP_DEFAULT_SCHEDULED \
		IScheduledThreadExecutorService::DefaultOnSubmitContract,\
		IScheduledThreadExecutorService::DefaultOnFutureContract,\
		IScheduledThreadExecutorService::DefaultOnRunContract,\
		IScheduledThreadExecutorService::DefaultOnCallContract, \
		IScheduledThreadExecutorService::DefaultOnScheduleRunContract,\
		IScheduledThreadExecutorService::DefaultOnScheduleFutureContract,\
		IScheduledThreadExecutorService::DefaultOnScheduleRepeatableContract

#endif

#ifndef NO_OP_DEFAULT_WORKSTEAL
#define  NO_OP_DEFAULT_WORKSTEAL \
	IWorkStealerService::DefaultOnInvokeContract, \
	IWorkStealerService::DefaultOnForkContract, \
	IWorkStealerService::DefaultOnJoinContract

#endif

	//Tags for IThreadExecutorService
	struct EventContractOnSubmit final {};
	struct EventContractOnFuture final {};
	struct EventContractOnRun final {};
	struct EventContractOnCall final {};

	//Tags for IScheduledThreadExecutorService
	struct EventContractOnScheduleRun final {};
	struct EventContractOnScheduleFuture final {};
	struct EventContractOnScheduleRepeatable final {};

	//Tags for IWorkStealerService
	struct EventContractOnInvoke final {};
	struct EventContractOnFork final {};
	struct EventContractOnJoin final {};

#if ENABLE_EVENT_EMITTERS_CORIUM
	template<typename D, typename E = void, size_t Hash = Events::DEFAULT_HASH>
		requires std::disjunction_v<Events::HasContract<E>, std::is_void<E>>
#else
	template<typename D, size_t Hash = Events::DEFAULT_HASH>
#endif
	class CORIUM IThreadExecutorService : public IExecutor<D>, public IExecutorVirtual {
		static_assert(!std::is_final_v<D>, "A class extending an executor service must be final");
		using Derived = D;
#if ENABLE_EVENT_EMITTERS_CORIUM
		using EventContract = std::conditional_t<!std::is_void_v<E>, E, void>;

		static_assert(!std::is_void_v<EventContract>&& EventContract::Count >= 4, "At least 4 event hook types must be provided in the EventContract");
		using SubmitEvent = std::tuple_element_t<0, typename EventContract::EventPack>;
		using FutureEvent = std::tuple_element_t<1, typename EventContract::EventPack>;
		using RunEvent = std::tuple_element_t<2, typename EventContract::EventPack>;
		using CallEvent = std::tuple_element_t<3, typename EventContract::EventPack>;
	public:
		using OnSubmit = Events::ICoriumEvent<EventContractOnSubmit, Hash>;
		using OnFuture = Events::ICoriumEvent<EventContractOnFuture, Hash>;
		using OnRun = Events::ICoriumEvent<EventContractOnRun, Hash>;
		using OnCall = Events::ICoriumEvent<EventContractOnCall, Hash>;

		struct DefaultOnFutureContract final : Events::ICoriumEvent<EventContractOnFuture, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnRunContract final : Events::ICoriumEvent<EventContractOnRun, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnSubmitContract final : Events::ICoriumEvent<EventContractOnSubmit, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnCallContract final : Events::ICoriumEvent<EventContractOnCall, Hash> {
			static void invoke() {/* Default implementation does nothing */ }
		};

		using DefaultPack = Events::EventEmitterPack<Hash, NO_OP_DEFAULT_INSTANTANEOUS>;
#endif
	public:
		static_assert(std::is_base_of_v<IThreadExecutorService, Derived>,
			"Derived is not a subclass of IThreadExecutorService");

		template<typename F, typename ...Args> requires(Traits::HasVariadicSubmitC<Derived, F, Args...>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> submit(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_args) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnSubmit, SubmitEvent>,
					"The first Event type must be a OnSubmit Hook");
				Events::ICoriumEvent<EventContractOnSubmit, Hash>::template invoke<SubmitEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template submitC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_args)...);
		}

		template<typename F> requires(Traits::HasSubmitC<Derived, F>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> submit(F&& u_Func, const Utils::TaskOptions& r_Options) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnSubmit, SubmitEvent>,
					"The first Event type must be a OnSubmit Hook");
				Events::ICoriumEvent<EventContractOnSubmit, Hash>::template invoke<SubmitEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template submitC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename...Args> requires(Traits::HasVariadicFutureC<Derived, F, Args...>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> future(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_args) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnFuture, FutureEvent>,
					"The second Event type must be a OnFuture Hook");
				Events::ICoriumEvent<EventContractOnFuture, Hash>::template invoke<FutureEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template futureC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_args)...);
		}

		template<typename F> requires(Traits::HasFutureC<Derived, F>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> future(F&& u_Func, const Utils::TaskOptions& r_Options) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnFuture, FutureEvent>,
					"The second Event type must be a OnFuture Hook");
				Events::ICoriumEvent<EventContractOnFuture, Hash>::template invoke<FutureEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template futureC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename...Args> requires(Traits::HasVariadicRunC<Derived, F, Args...>)
			void run(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnRun, RunEvent>,
					"The third Event type must be a OnRun Hook");
				Events::ICoriumEvent<EventContractOnRun, Hash>::template invoke<RunEvent>();
			}
#endif
			static_cast<Derived*>(this)->template runC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename F> requires(Traits::HasRunC<Derived, F>)
			void run(F&& u_Func, const Utils::TaskOptions& r_Options) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnRun, RunEvent>,
					"The third Event type must be a OnRun Hook");
				Events::ICoriumEvent<EventContractOnRun, Hash>::template invoke<RunEvent>();
			}
#endif
			static_cast<Derived*>(this)->template runC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename...Args> requires(Traits::HasVariadicCallC<Derived, F, Args...>)
			[[nodiscard]] decltype(auto) call(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnCall, CallEvent>,
					"The fourth Event type must be a OnCall Hook");
				Events::ICoriumEvent<EventContractOnCall, Hash>::template invoke<CallEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template callC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename F> requires(Traits::HasCallC<Derived, F>)
			[[nodiscard]] decltype(auto) call(F&& u_Func, const Utils::TaskOptions& r_Options) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnCall, CallEvent>,
					"The fourth type must be a OnCall Hook");
				Events::ICoriumEvent<EventContractOnCall, Hash>::template invoke<CallEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template callC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, size_t N> requires(Traits::HasSubmitBatchC<Derived, F, N>)
			[[nodiscard]] std::array<Memory::SharedPointer<Utils::IHandle>, N>
			submitBatch(F(&& u_Func)[N], const Utils::TaskOptions(&ra_Options)[N]) {
			return static_cast<Derived*>(this)->template submitBatchC<F>(std::move(u_Func), ra_Options);
		}

		template<typename F, size_t N> requires(Traits::HasFutureBatchC<Derived, F, N>)
			[[nodiscard]] std::array<Memory::SharedPointer<Utils::IHandle>, N>
			futureBatch(F(&& u_Func)[N], const Utils::TaskOptions(&ra_Options)[N]) {
			return static_cast<Derived*>(this)->template futureBatchC<F>(std::move(u_Func), ra_Options);
		}
	};

#if ENABLE_EVENT_EMITTERS_CORIUM
	template<typename D, typename E = void, size_t Hash = Events::DEFAULT_HASH>
		requires std::disjunction_v<Events::HasContract<E>, std::is_void<E>>
#else
	template<typename D, size_t Hash = Events::DEFAULT_HASH>
#endif
	class CORIUM IScheduledThreadExecutorService : public IExecutor<D>, public IExecutorVirtual {
		using Derived = D;
		using EventContract = E;
		static_assert(!std::is_final_v<D>, "A class extending an executor service must be final");

#if ENABLE_EVENT_EMITTERS_CORIUM
		static_assert(!std::is_void_v<EventContract>&& EventContract::Count >= 7, "At least 7 event hooks must be present in the EventContract");
		using SubmitEvent = std::tuple_element_t<0, typename EventContract::EventPack>;
		using FutureEvent = std::tuple_element_t<1, typename EventContract::EventPack>;
		using RunEvent = std::tuple_element_t<2, typename EventContract::EventPack>;
		using CallEvent = std::tuple_element_t<3, typename EventContract::EventPack>;

		using ScheduleRunEvent = std::tuple_element_t<4, typename EventContract::EventPack>;
		using ScheduleFutureEvent = std::tuple_element_t<5, typename EventContract::EventPack>;
		using ScheduleRepeatableEvent = std::tuple_element_t<6, typename EventContract::EventPack>;

	public:
		using OnSubmit = Events::ICoriumEvent<EventContractOnSubmit, Hash>;
		using OnFuture = Events::ICoriumEvent<EventContractOnFuture, Hash>;
		using OnRun = Events::ICoriumEvent<EventContractOnRun, Hash>;
		using OnCall = Events::ICoriumEvent<EventContractOnCall, Hash>;

		using OnScheduleRun = Events::ICoriumEvent<EventContractOnScheduleRun, Hash>;
		using OnScheduleFuture = Events::ICoriumEvent<EventContractOnScheduleFuture, Hash>;
		using OnScheduleRepeatable = Events::ICoriumEvent<EventContractOnScheduleRepeatable, Hash>;

		struct DefaultOnFutureContract final : Events::ICoriumEvent<EventContractOnFuture, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnRunContract final : Events::ICoriumEvent<EventContractOnRun, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnSubmitContract final : Events::ICoriumEvent<EventContractOnSubmit, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnScheduleRunContract final : Events::ICoriumEvent<EventContractOnScheduleRun, Hash> {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnScheduleFutureContract final : Events::ICoriumEvent<EventContractOnScheduleFuture, Hash> {
			static void invoke() { /* Default implementation does nothing */ }
		};

		struct DefaultOnScheduleRepeatableContract final : Events::ICoriumEvent<EventContractOnScheduleRepeatable, Hash> {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnCallContract final : Events::ICoriumEvent<EventContractOnCall, Hash> {
			static void invoke() {/* Default implementation does nothing */ }
		};

		using DefaultPack = Events::EventEmitterPack<Hash, NO_OP_DEFAULT_SCHEDULED>;
#endif
	private:
		using Duration = std::chrono::steady_clock::duration;
		using TimePoint = std::chrono::steady_clock::time_point;
	public:
		template<typename F, typename...Args> requires(Traits::HasVariadicSubmitC<Derived, F, Args...>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> submit(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnSubmit, SubmitEvent>,
					"The first Event type must be a OnSubmit Hook");
				Events::ICoriumEvent<EventContractOnSubmit, Hash>::template invoke<SubmitEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template submitC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template< typename F, typename T, typename...Args > requires(Traits::HasVariadicScheduleRunC<Derived, F, T, Args...> && Traits::IsDurationV<T>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> scheduleRun(F&& u_Func, const Utils::TaskOptions& r_Options, T&& u_Duration, Args&&...u_Args) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnScheduleRun, ScheduleRunEvent>,
					"The fifth Event type must be a OnScheduleRun Hook");
				Events::ICoriumEvent<EventContractOnScheduleRun, Hash>::template invoke<ScheduleRunEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template scheduleRunC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<T>(u_Duration), std::forward<Args>(u_Args)...);
		}

		template<typename F> requires(Traits::HasSubmitC<Derived, F>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> submit(F&& u_Func, const Utils::TaskOptions& r_Options) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnSubmit, SubmitEvent>,
					"The first Event type must be a OnSubmit Hook");
				Events::ICoriumEvent<EventContractOnSubmit, Hash>::template invoke<SubmitEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template submitC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename T> requires(Traits::HasScheduleRunC<Derived, F, T> && Traits::IsDurationV<T>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> scheduleRun(F&& u_Func, const Utils::TaskOptions& r_Options, T&& u_Duration) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnScheduleRun, ScheduleRunEvent>,
					"The fifth Event type must be a OnScheduleRun Hook");
				Events::ICoriumEvent<EventContractOnScheduleRun, Hash>::template invoke<ScheduleRunEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template scheduleRunC<F>(std::forward<F>(u_Func), r_Options, std::forward<T>(u_Duration));
		}

		template<typename F, typename...Args> requires(Traits::HasVariadicRunC<Derived, F, Args...>)
			void run(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnRun, RunEvent>,
					"The third Event type must be a OnRun Hook");
				Events::ICoriumEvent<EventContractOnRun, Hash>::template invoke<RunEvent>();
			}
#endif
			static_cast<Derived*>(this)->template runC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename F> requires(Traits::HasRunC<Derived, F>)
			void run(F&& u_Func, const Utils::TaskOptions& r_Options) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnRun, RunEvent>,
					"The third Event type must be a OnRun Hook");
				Events::ICoriumEvent<EventContractOnRun, Hash>::template invoke<RunEvent>();
			}
#endif
			static_cast<Derived*>(this)->template runC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename...Args> requires(Traits::HasVariadicFutureC<Derived, F, Args...>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> future(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnFuture, FutureEvent>,
					"The second Event type must be a OnFuture Hook");
				Events::ICoriumEvent<EventContractOnFuture, Hash>::template invoke<FutureEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template futureC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename F, typename T, typename...Args> requires(Traits::HasVariadicScheduleFutureC<D, F, T, Args...> && Traits::IsDurationV<T>)
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> scheduleFuture(F&& u_Func, const Utils::TaskOptions& r_Options, T&& u_Duration, Args&&... u_Args) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnScheduleFuture, ScheduleFutureEvent>,
					"The sixth Event type must be a OnScheduleFuture Hook");
				Events::ICoriumEvent<EventContractOnScheduleFuture, Hash>::template invoke<ScheduleFutureEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template scheduleFutureC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<T>(u_Duration), std::forward<Args>(u_Args)...);
		}

		template<typename F> requires(Traits::HasFutureC<Derived, F>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> future(F&& u_Func, const Utils::TaskOptions& r_Options) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnFuture, FutureEvent>,
					"The second Event type must be a OnFuture Hook");
				Events::ICoriumEvent<EventContractOnFuture, Hash>::template invoke<FutureEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template futureC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename T> requires(Traits::HasScheduleFutureC<Derived, F, T> && Traits::IsDurationV<T>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> scheduleFuture(F&& u_Func, const Utils::TaskOptions& r_Options, T&& u_Duration) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnScheduleFuture, ScheduleFutureEvent>,
					"The sixth Event type must be a OnScheduleFuture Hook");
				Events::ICoriumEvent<EventContractOnScheduleFuture, Hash>::template invoke<ScheduleFutureEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template scheduleFutureC<F>(std::forward<F>(u_Func), r_Options, std::forward<T>(u_Duration));
		}

		template<typename F, typename...Args> requires(Traits::HasVariadicCallC<Derived, F, Args...>)
			[[nodiscard]] decltype(auto) call(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnCall, CallEvent>,
					"The fourth Event type must be a OnCall Hook");
				Events::ICoriumEvent<EventContractOnCall, Hash>::template invoke<CallEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template callC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename F> requires(Traits::HasCallC<Derived, F>)
			[[nodiscard]] decltype(auto) call(F&& u_Func, const Utils::TaskOptions& r_Options) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnCall, CallEvent>,
					"The fourth type must be a OnCall Hook");
				Events::ICoriumEvent<EventContractOnCall, Hash>::template invoke<CallEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template callC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, size_t N> requires(Traits::HasSubmitBatchC<Derived, F, N>)
			[[nodiscard]] std::array<Memory::SharedPointer<Utils::IHandle>, N>
			submitBatch(F(&& u_Func)[N], const Utils::TaskOptions(&ra_Options)[N]) {
			return static_cast<Derived*>(this)->template submitBatchC<F>(std::move(u_Func), ra_Options);
		}

		template<typename F, size_t N> requires(Traits::HasFutureBatchC<Derived, F, N>)
			[[nodiscard]] std::array<Memory::SharedPointer<Utils::IHandle>, N>
			futureBatch(F(&& u_Func)[N], const Utils::TaskOptions(&ra_Options)[N]) {
			return static_cast<Derived*>(this)->template futureBatchC<F>(std::move(u_Func), ra_Options);
		}

		template<typename F> requires(Traits::HasScheduleRepeatableC<Derived, F>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> scheduleRepeatable(F&& u_Func, const Utils::TaskOptions& r_Options) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnScheduleRepeatable, ScheduleRepeatableEvent>,
					"The seventh Event type must be a OnScheduleRepeatable Hook");
				Events::ICoriumEvent<EventContractOnScheduleRepeatable, Hash>::template invoke<ScheduleRepeatableEvent>();
			}
#endif
			return static_cast<Derived*>(this)->template scheduleRepeatableC<F>(std::forward<F>(u_Func), r_Options);
		}

		[[nodiscard]] virtual size_t getScheduledTaskCount() const noexcept = 0;
		virtual bool cancelScheduled() const noexcept = 0;

		template<typename T> requires Traits::IsDurationV<T>
		void delayedShutdown(T&& u_Duration) {
			static_cast<Derived*>(this)->delayedShutdownC(std::forward<T>(u_Duration));
		}
	};

#if ENABLE_EVENT_EMITTERS_CORIUM
	template<typename D, typename E = void, size_t Hash = Events::DEFAULT_HASH>
		requires std::disjunction_v<Events::HasContract<E>, std::is_void<E>>
#else
	template<typename D, size_t Hash = Events::DEFAULT_HASH>
#endif
	class CORIUM IWorkStealerService : IExecutor<D> {
		using Derived = D;
		using EventContract = E;
		static_assert(!std::is_final_v<Derived> && !std::is_base_of_v<IWorkStealerService, Derived>,
			"The Derived class must be final and should extend IWorkStealerService");

#if ENABLE_EVENT_EMITTERS_CORIUM
		using ForkEvent = std::tuple_element_t<0, typename EventContract::Contract>;
		using JoinEvent = std::tuple_element_t<1, typename EventContract::Contract>;
		using InvokeEvent = std::tuple_element_t <2, typename EventContract::Contract>;
	public:
		using OnFork = Events::ICoriumEvent<EventContractOnFork, Hash>;
		using OnJoin = Events::ICoriumEvent<EventContractOnJoin, Hash>;
		using OnInvoke = Events::ICoriumEvent<EventContractOnInvoke, Hash>;

		using DefaultPack = Events::EventEmitterPack<Hash, NO_OP_DEFAULT_WORKSTEAL>;

		struct DefaultOnForkContract final : OnFork {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnJoinContract final : OnJoin {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnInvokeContract final : OnInvoke {
			static void invoke() {/* Default implementation does nothing */ }
		};
#endif
	public:
		template<typename T>
		Memory::SharedPointer<Utils::IHandle> invoke(const T& r_Workload) {
			static_assert(std::is_final_v<T> && !std::is_base_of_v<WorkSteal::IWorkStealLoad<T, E, Hash>, T>,
				"The task submitted must be a subclass of IWorkStealLoad");
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v<InvokeEvent, EventContractOnInvoke>,
					"The first Event type must be OnInvoke hook");
				Events::ICoriumEvent<EventContractOnInvoke, Hash>::template invoke<InvokeEvent>();
			}
#endif
			return static_cast<Derived*>(this) ->template invokeC<T>(r_Workload);
		}

		template<typename T>
		Memory::SharedPointer<Utils::IHandle> fork(const T&& u_Workload) {
			static_assert(std::is_final_v<T> && !std::is_base_of_v<WorkSteal::IWorkStealLoad<T, E, Hash>, T>,
				"The task submitted must be a subclass of IWorkStealLoad");
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v<ForkEvent, EventContractOnFork>,
					"The second Event type must be OnFork hook");
				Events::ICoriumEvent<EventContractOnFork, Hash>::template invoke<ForkEvent>();
			}
#endif
			return static_cast<Derived*>(this) ->template forkC<T>(u_Workload);
		}

		void join(Memory::SharedPointer<Utils::IHandle> ss_Handle) {
#if defined(ENABLE_EVENT_EMITTERS_CORIUM)
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v<JoinEvent, EventContractOnJoin>,
					"The third Event type must be OnJoin hook");
				Events::ICoriumEvent<EventContractOnJoin, Hash>::template invoke<JoinEvent>();
			}
#endif
			static_cast<Derived*>(this)->joinC(ss_Handle);
		}

		virtual ~IWorkStealerService() = default;

		[[nodiscard]] virtual bool isRunning() const noexcept = 0;

		[[nodiscard]] virtual bool isShutdown() const noexcept = 0;

		virtual void shutdown() noexcept = 0;
		virtual void shutdownNow() noexcept = 0;
		virtual bool cancelAllPending() noexcept = 0;

		[[nodiscard]] virtual bool isQuiescent() const noexcept = 0;
		[[nodiscard]] virtual size_t getActiveTaskCount() const noexcept = 0;
		[[nodiscard]] virtual size_t getCompletedTasksCount() const noexcept = 0;
		[[nodiscard]] virtual size_t getCancelledTasksCount() const noexcept = 0;
	};
}
