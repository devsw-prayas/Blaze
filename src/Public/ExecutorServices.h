#pragma once
#include "Blaze.h"
#include "BlazeEvent.h"
#include "BlazeTraits.h"
#include "Executors.h"

namespace Blaze::Executors::Services {
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

	//Tags for IThreadExecutorService
	struct EventContractOnSubmit final {};
	struct EventContractOnFuture final {};
	struct EventContractOnRun final {};
	struct EventContractOnCall final {};

	//Tags for IScheduledThreadExecutorService
	struct EventContractOnScheduleRun final {};
	struct EventContractOnScheduleFuture final {};
	struct EventContractOnScheduleRepeatable final {};

	template<typename Derived, typename EventContract = std::conditional_t<ENABLE_EVENT_EMITTERS_BLAZE, Events::EventEmitterPack<>, void>,
		size_t Hash = Events::DEFAULT_HASH>
	class BLAZE IThreadExecutorService : public IExecutor<Derived>, public IExecutorVirtual {
#if ENABLE_EVENT_EMITTERS_BLAZE
		static_assert(!std::is_void_v<EventContract>&& EventContract::Count >= 4, "At least 4 event hook types must be provided in the EventContract");

		using OnSubmit = Events::IBlazeEvent<EventContractOnSubmit, Hash>;
		using OnFuture = Events::IBlazeEvent<EventContractOnFuture, Hash>;
		using OnRun = Events::IBlazeEvent<EventContractOnRun, Hash>;
		using OnCall = Events::IBlazeEvent<EventContractOnCall, Hash>;

		using SubmitEvent = std::tuple_element_t<0, typename EventContract::EventPack>;
		using FutureEvent = std::tuple_element_t<1, typename EventContract::EventPack>;
		using RunEvent = std::tuple_element_t<2, typename EventContract::EventPack>;
		using CallEvent = std::tuple_element_t<3, typename EventContract::EventPack>;
	public:
		struct DefaultOnFutureContract final : Events::IBlazeEvent<EventContractOnFuture, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnRunContract final : Events::IBlazeEvent<EventContractOnRun, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnSubmitContract final : Events::IBlazeEvent<EventContractOnSubmit, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnCallContract final : Events::IBlazeEvent<EventContractOnCall, Hash> {
			static void invoke() {/* Default implementation does nothing */ }
		};

		using DefaultPack = Events::EventEmitterPack<Hash, NO_OP_DEFAULT_INSTANTANEOUS>;
#endif
	public:
		static_assert(std::is_base_of_v<IThreadExecutorService, Derived>,
			"Derived is not a subclass of IThreadExecutorService");

		template<typename F, typename ...Args> requires(Traits::HasVariadicSubmitC<Derived, F, Args...>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> submit(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_args) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnSubmit, SubmitEvent>,
					"The first Event type must be a OnSubmit Hook");
				Events::IBlazeEvent<EventContractOnSubmit, Hash>::template invoke<SubmitEvent>();
			}
			return static_cast<Derived*>(this)->template submitC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_args)...);
		}

		template<typename F> requires(Traits::HasSubmitC<Derived, F>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> submit(F&& u_Func, const Utils::TaskOptions& r_Options) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnSubmit, SubmitEvent>,
					"The first Event type must be a OnSubmit Hook");
				Events::IBlazeEvent<EventContractOnSubmit, Hash>::template invoke<SubmitEvent>();
			}
			return static_cast<Derived*>(this)->template submitC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename...Args> requires(Traits::HasVariadicFutureC<Derived, F, Args...>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> future(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_args) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnFuture, FutureEvent>,
					"The second Event type must be a OnFuture Hook");
				Events::IBlazeEvent<EventContractOnFuture, Hash>::template invoke<FutureEvent>();
			}
			return static_cast<Derived*>(this)->template futureC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_args)...);
		}

		template<typename F> requires(Traits::HasFutureC<Derived, F>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> future(F&& u_Func, const Utils::TaskOptions& r_Options) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnFuture, FutureEvent>,
					"The second Event type must be a OnFuture Hook");
				Events::IBlazeEvent<EventContractOnFuture, Hash>::template invoke<FutureEvent>();
			}
			return static_cast<Derived*>(this)->template futureC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename...Args> requires(Traits::HasVariadicRunC<Derived, F, Args...>)
			void run(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnRun, RunEvent>,
					"The third Event type must be a OnRun Hook");
				Events::IBlazeEvent<EventContractOnRun, Hash>::template invoke<RunEvent>();
			}
			static_cast<Derived*>(this)->template runC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename F> requires(Traits::HasRunC<Derived, F>)
			void run(F&& u_Func, const Utils::TaskOptions& r_Options) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnRun, RunEvent>,
					"The third Event type must be a OnRun Hook");
				Events::IBlazeEvent<EventContractOnRun, Hash>::template invoke<RunEvent>();
			}
			static_cast<Derived*>(this)->template runC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename...Args> requires(Traits::HasVariadicCallC<Derived, F, Args...>)
			[[nodiscard]] decltype(auto) call(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnCall, CallEvent>,
					"The fourth Event type must be a OnCall Hook");
				Events::IBlazeEvent<EventContractOnCall, Hash>::template invoke<CallEvent>();
			}
			return static_cast<Derived*>(this)->template callC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename F> requires(Traits::HasCallC<Derived, F>)
			[[nodiscard]] decltype(auto) call(F&& u_Func, const Utils::TaskOptions& r_Options) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnCall, CallEvent>,
					"The fourth type must be a OnCall Hook");
				Events::IBlazeEvent<EventContractOnCall, Hash>::template invoke<CallEvent>();
			}
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

	template<typename Derived, typename EventContract = std::conditional_t<ENABLE_EVENT_EMITTERS_BLAZE, Events::EventEmitterPack<>, void>,
		size_t Hash = Events::DEFAULT_HASH>
	class IScheduledThreadExecutorService : public IExecutor<Derived>, public IExecutorVirtual {
#if ENABLE_EVENT_EMITTERS_BLAZE
		static_assert(!std::is_void_v<EventContract>&& EventContract::Count >= 7, "At least 7 event hooks must be present in the EventContract");
		using OnSubmit = Events::IBlazeEvent<EventContractOnSubmit, Hash>;
		using OnFuture = Events::IBlazeEvent<EventContractOnFuture, Hash>;
		using OnRun = Events::IBlazeEvent<EventContractOnRun, Hash>;
		using OnCall = Events::IBlazeEvent<EventContractOnCall, Hash>;

		using OnScheduleRun = Events::IBlazeEvent<EventContractOnScheduleRun, Hash>;
		using OnScheduleFuture = Events::IBlazeEvent<EventContractOnScheduleFuture, Hash>;
		using OnScheduleRepeatable = Events::IBlazeEvent<EventContractOnScheduleRepeatable, Hash>;

		using SubmitEvent = std::tuple_element_t<0, typename EventContract::EventPack>;
		using FutureEvent = std::tuple_element_t<1, typename EventContract::EventPack>;
		using RunEvent = std::tuple_element_t<2, typename EventContract::EventPack>;
		using CallEvent = std::tuple_element_t<3, typename EventContract::EventPack>;

		using ScheduleRunEvent = std::tuple_element_t<4, typename EventContract::EventPack>;
		using ScheduleFutureEvent = std::tuple_element_t<5, typename EventContract::EventPack>;
		using ScheduleRepeatableEvent = std::tuple_element_t<6, typename EventContract::EventPack>;

	public:
		struct DefaultOnFutureContract final : Events::IBlazeEvent<EventContractOnFuture, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnRunContract final : Events::IBlazeEvent<EventContractOnRun, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnSubmitContract final : Events::IBlazeEvent<EventContractOnSubmit, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnScheduleRunContract final : Events::IBlazeEvent<EventContractOnScheduleRun, Hash> {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnScheduleFutureContract final : Events::IBlazeEvent<EventContractOnScheduleFuture, Hash> {
			static void invoke() { /* Default implementation does nothing */ }
		};

		struct DefaultOnScheduleRepeatableContract final : Events::IBlazeEvent<EventContractOnScheduleRepeatable, Hash> {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnCallContract final : Events::IBlazeEvent<EventContractOnCall, Hash> {
			static void invoke() {/* Default implementation does nothing */ }
		};

		using DefaultPack = Events::EventEmitterPack<Hash, NO_OP_DEFAULT_SCHEDULED>;
#endif
	public:
		template<typename F, typename...Args> requires(Traits::HasVariadicSubmitC<Derived, F, Args...>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> submit(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnSubmit, SubmitEvent>,
					"The first Event type must be a OnSubmit Hook");
				Events::IBlazeEvent<EventContractOnSubmit, Hash>::template invoke<SubmitEvent>();
			}
			return static_cast<Derived*>(this)->template submitC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template< typename F, typename...Args > requires(Traits::HasVariadicScheduleRunC<Derived, F, Args...>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> scheduleRun(F&& u_Func, const Utils::TaskOptions& r_Options, auto&& u_Duration, Args&&...u_Args) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnScheduleRun, ScheduleRunEvent>,
					"The fifth Event type must be a OnScheduleRun Hook");
				Events::IBlazeEvent<EventContractOnScheduleRun, Hash>::template invoke<ScheduleRunEvent>();
			}
			return static_cast<Derived*>(this)->template scheduleRunC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<decltype(u_Duration)>(u_Duration), std::forward<Args>(u_Args)...);
		}

		template<typename F> requires(Traits::HasSubmitC<Derived, F>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> submit(F&& u_Func, const Utils::TaskOptions& r_Options) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnSubmit, SubmitEvent>,
					"The first Event type must be a OnSubmit Hook");
				Events::IBlazeEvent<EventContractOnSubmit, Hash>::template invoke<SubmitEvent>();
			}
			return static_cast<Derived*>(this)->template submitC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F> requires(Traits::HasScheduleRunC<Derived, F>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> scheduleRun(F&& u_Func, const Utils::TaskOptions& r_Options, auto&& u_Duration) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnScheduleRun, ScheduleRunEvent>,
					"The fifth Event type must be a OnScheduleRun Hook");
				Events::IBlazeEvent<EventContractOnScheduleRun, Hash>::template invoke<ScheduleRunEvent>();
			}
			return static_cast<Derived*>(this)->template scheduleRunC<F>(std::forward<F>(u_Func), r_Options, std::forward<decltype(u_Duration)>(u_Duration));
		}

		template<typename F, typename...Args> requires(Traits::HasVariadicRunC<Derived, F, Args...>)
			void run(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnRun, RunEvent>,
					"The third Event type must be a OnRun Hook");
				Events::IBlazeEvent<EventContractOnRun, Hash>::template invoke<RunEvent>();
			}
			static_cast<Derived*>(this)->template runC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename F> requires(Traits::HasRunC<Derived, F>)
			void run(F&& u_Func, const Utils::TaskOptions& r_Options) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnRun, RunEvent>,
					"The third Event type must be a OnRun Hook");
				Events::IBlazeEvent<EventContractOnRun, Hash>::template invoke<RunEvent>();
			}
			static_cast<Derived*>(this)->template runC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename...Args> requires(Traits::HasVariadicFutureC<Derived, F, Args...>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> future(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnFuture, FutureEvent>,
					"The second Event type must be a OnFuture Hook");
				Events::IBlazeEvent<EventContractOnFuture, Hash>::template invoke<FutureEvent>();
			}
			return static_cast<Derived*>(this)->template futureC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename F, typename...Args>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> scheduleFuture(F&& u_Func, const Utils::TaskOptions& r_Options, auto&& u_Duration, Args&&... u_Args) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnScheduleFuture, ScheduleFutureEvent>,
					"The sixth Event type must be a OnScheduleFuture Hook");
				Events::IBlazeEvent<EventContractOnScheduleFuture, Hash>::template invoke<ScheduleFutureEvent>();
			}
			return static_cast<Derived*>(this)->template scheduleFutureC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<decltype(u_Duration)>(u_Duration), std::forward<Args>(u_Args)...);
		}

		template<typename F> requires(Traits::HasFutureC<Derived, F>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> future(F&& u_Func, const Utils::TaskOptions& r_Options) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnFuture, FutureEvent>,
					"The second Event type must be a OnFuture Hook");
				Events::IBlazeEvent<EventContractOnFuture, Hash>::template invoke<FutureEvent>();
			}
			return static_cast<Derived*>(this)->template futureC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F> requires(Traits::HasScheduleFutureC<Derived, F>)
			[[nodiscard]] Memory::SharedPointer<Utils::IHandle> scheduleFuture(F&& u_Func, const Utils::TaskOptions& r_Options, auto&& u_Duration) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnScheduleFuture, ScheduleFutureEvent>,
					"The sixth Event type must be a OnScheduleFuture Hook");
				Events::IBlazeEvent<EventContractOnScheduleFuture, Hash>::template invoke<ScheduleFutureEvent>();
			}
			return static_cast<Derived*>(this)->template scheduleFutureC<F>(std::forward<F>(u_Func), r_Options, std::forward<decltype(u_Duration)>(u_Duration));
		}

		template<typename F, typename...Args> requires(Traits::HasVariadicCallC<Derived, F, Args...>)
			[[nodiscard]] decltype(auto) call(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnCall, CallEvent>,
					"The fourth Event type must be a OnCall Hook");
				Events::IBlazeEvent<EventContractOnCall, Hash>::template invoke<CallEvent>();
			}
			return static_cast<Derived*>(this)->template callC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename F> requires(Traits::HasCallC<Derived, F>)
			[[nodiscard]] decltype(auto) call(F&& u_Func, const Utils::TaskOptions& r_Options) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnCall, CallEvent>,
					"The fourth type must be a OnCall Hook");
				Events::IBlazeEvent<EventContractOnCall, Hash>::template invoke<CallEvent>();
			}
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
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnScheduleRepeatable, ScheduleRepeatableEvent>,
					"The seventh Event type must be a OnScheduleRepeatable Hook");
				Events::IBlazeEvent<EventContractOnScheduleRepeatable, Hash>::template invoke<ScheduleRepeatableEvent>();
			}
			return static_cast<Derived*>(this)->template scheduleRepeatableC<F>(std::forward<F>(u_Func), r_Options);
		}

		[[nodiscard]] virtual size_t getScheduledTaskCount() const noexcept = 0;
		virtual bool cancelScheduled() const noexcept = 0;

		void delayedShutdown(auto&& u_Duration) {
			static_cast<Derived*>(this)->delayedShutdownC(std::forward<decltype(u_Duration)>(u_Duration));
		}
	};
}
