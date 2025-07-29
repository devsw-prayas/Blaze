#pragma once
#include "Blaze.h"
#include "BlazeEvent.h"
#include "Executors.h"

namespace Blaze::Executors::Services {
#ifndef NO_OP_DEFAULT_INSTANTANEOUS
#define NO_OP_DEFAULT_INSTANTANEOUS \
	IThreadExecutorService::DefaultOnSubmitContract, IThreadExecutorService::DefaultOnCallContract, IThreadExecutorService::DefaultOnRunContract
#endif

	struct EventContractOnSubmit final {};
	struct EventContractOnCall final {};
	struct EventContractOnRun final {};

	template<typename Derived, typename EventContract = std::conditional_t<ENABLE_EVENT_EMITTERS_BLAZE, Events::EventEmitterPack<>, void>,
		size_t Hash = Events::DEFAULT_HASH>
	class BLAZE IThreadExecutorService : IExecutor<Derived>, IExecutorVirtual {
	public:
		static_assert(!std::is_void_v<EventContract>&& EventContract::Count >= 3, "At least 3 event hook types must be provided in the EventContract");
#if ENABLE_EVENT_EMITTERS_BLAZE
	private:
		using OnSubmit = Events::IBlazeEvent<EventContractOnSubmit, Hash>;
		using OnCall = Events::IBlazeEvent<EventContractOnCall, Hash>;
		using OnRun = Events::IBlazeEvent<EventContractOnRun, Hash>;

		using SubmitEvent = std::tuple_element_t<0, typename EventContract::EventPack>;
		using CallEvent = std::tuple_element_t<1, typename EventContract::EventPack>;
		using RunEvent = std::tuple_element_t<2, typename EventContract::EventPack>;
	public:
		struct DefaultOnCallContract final : Events::IBlazeEvent<EventContractOnCall, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnRunContract final : Events::IBlazeEvent<EventContractOnRun, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};

		struct DefaultOnSubmitContract final : Events::IBlazeEvent<EventContractOnSubmit, Hash > {
			static void invoke() {/* Default implementation does nothing */ }
		};
#endif
		static_assert(std::is_base_of_v<IThreadExecutorService, Derived>,
			"Derived is not a subclass of IThreadExecutorService");

		template<typename F, typename ...Args>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> submit(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_args) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnSubmit, SubmitEvent>,
					"The first Event type must be a OnSubmit Hook");
				Events::IBlazeEvent<EventContractOnSubmit, Hash>::template invoke<SubmitEvent>();
			}
			return static_cast<Derived*>(this)->template submitC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_args)...);
		}

		template<typename F>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> submit(F&& u_Func, const Utils::TaskOptions& r_Options) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnSubmit, SubmitEvent>,
					"The first Event type must be a OnSubmit Hook");
				Events::IBlazeEvent<EventContractOnSubmit, Hash>::template invoke<SubmitEvent>();
			}
			return static_cast<Derived*>(this)->template submitC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename...Args>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> call(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_args) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnCall, CallEvent>,
					"The second Event type must be a OnCall Hook");
				Events::IBlazeEvent<EventContractOnCall, Hash>::template invoke<CallEvent>();
			}
			return static_cast<Derived*>(this)->template callC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_args)...);
		}

		template<typename F>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> call(F&& u_Func, const Utils::TaskOptions& r_Options) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnCall, CallEvent>,
					"The second Event type must be a OnCall Hook");
				Events::IBlazeEvent<EventContractOnCall, Hash>::template invoke<CallEvent>();
			}
			return static_cast<Derived*>(this)->template callC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename...Args>
		void run(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnRun, RunEvent>,
					"The third Event type must be a OnRun Hook");
				Events::IBlazeEvent<EventContractOnRun, Hash>::template invoke<RunEvent>();
			}
			static_cast<Derived*>(this)->template runC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename F>
		void run(F&& u_Func, const Utils::TaskOptions& r_Options) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnRun, RunEvent>,
					"The third Event type must be a OnRun Hook");
				Events::IBlazeEvent<EventContractOnRun, Hash>::template invoke<RunEvent>();
			}
			static_cast<Derived*>(this)->template runC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename T, typename F, typename...Args>
		[[nodiscard]] T&& call(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnCall, CallEvent>,
					"The second Event type must be a OnCall Hook");
				Events::IBlazeEvent<EventContractOnCall, Hash>::template invoke<CallEvent>();
			}
			return static_cast<Derived*>(this)->template callC<T, F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename T, typename F>
		[[nodiscard]] T&& call(F&& u_Func, const Utils::TaskOptions& r_Options) {
			if constexpr (!std::is_void_v<EventContract>) {
				static_assert(std::is_base_of_v <OnCall, CallEvent>,
					"The second Event type must be a OnCall Hook");
				Events::IBlazeEvent<EventContractOnCall, Hash>::template invoke<CallEvent>();
			}
			return static_cast<Derived*>(this)->template callC<T, F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, size_t N>
		std::tuple<Memory::SharedPointer<Utils::IHandle>>
			submitBatch(F(&& u_Func)[N], const Utils::TaskOptions(&ra_Options)[N]) {
			return static_cast<Derived*>(this)->template submitBatchC<F>(std::move(u_Func), std::move(ra_Options));
		}

		template<typename F, size_t N>
		std::tuple<Memory::SharedPointer<Utils::IHandle>>
			callBatch(F(&& u_Func)[N], const Utils::TaskOptions(&ra_Options)[N]) {
			return static_cast<Derived*>(this)->template callBatchC<F>(std::move(u_Func), std::move(ra_Options));
		}
	};

	template<typename Derived, typename EventContract = std::conditional_t<ENABLE_EVENT_EMITTERS_BLAZE, Events::EventEmitterPack<>, void>,
		size_t Hash = Events::DEFAULT_HASH>
	class IScheduledThreadExecutorService : IExecutor<Derived>, IExecutorVirtual {
	public:
		template<typename F, typename...Args>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> submit(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
			return static_cast<Derived*>(this)->template submitC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template< typename F, typename...Args >
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> scheduleRun(F&& u_Func, const Utils::TaskOptions& r_Options, auto&& u_Duration, Args&&...u_Args) {
			return static_cast<Derived*>(this)->template scheduleRunC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<decltype(u_Duration)>(u_Duration), std::forward<Args>(u_Args)...);
		}

		template<typename F>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> submit(F&& u_Func, const Utils::TaskOptions& r_Options) {
			return static_cast<Derived*>(this)->template submitC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> scheduleRun(F&& u_Func, const Utils::TaskOptions& r_Options, auto&& u_Duration) {
			return static_cast<Derived*>(this)->template scheduleRunC<F>(std::forward<F>(u_Func), r_Options, std::forward<decltype(u_Duration)>(u_Duration));
		}

		template<typename F, typename...Args>
		void run(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
			static_cast<Derived*>(this)->template runC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename F>
		void run(F&& u_Func, const Utils::TaskOptions& r_Options) {
			static_cast<Derived*>(this)->template runC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename...Args>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> call(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
			return static_cast<Derived*>(this)->template callC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename F, typename...Args>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> scheduleCall(F&& u_Func, const Utils::TaskOptions& r_Options, auto&& u_Duration, Args&&... u_Args) {
			return static_cast<Derived*>(this)->template scheduleCallC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<decltype(u_Duration)>(u_Duration), std::forward<Args>(u_Args)...);
		}

		template<typename F>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> call(F&& u_Func, const Utils::TaskOptions& r_Options) {
			return static_cast<Derived*>(this)->template callC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> scheduleCall(F&& u_Func, const Utils::TaskOptions& r_Options, auto&& u_Duration) {
			return static_cast<Derived*>(this)->template scheduleCallC<F>(std::forward<F>(u_Func), r_Options, std::forward<decltype(u_Duration)>(u_Duration));
		}

		template<typename F,typename T, typename ...Args>
		[[nodiscard]] T&& call(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
			return static_cast<Derived*>(this)->template callC<T, F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename F, typename T>
		[[nodiscard]] T&& call(F&& u_Func, const Utils::TaskOptions& r_Options) {
			return static_cast<Derived*>(this)->template callC<T, F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, size_t N>
		std::tuple<Memory::SharedPointer<Utils::IHandle>>
			submitBatch(F(&& u_Func)[N], const Utils::TaskOptions(&ra_Options)[N]) {
			return static_cast<Derived*>(this)->template submitBatchC<F>(std::move(u_Func), std::move(ra_Options));
		}

		template<typename F, size_t N>
		[[nodiscard]] std::tuple<Memory::SharedPointer<Utils::IHandle>>
			callBatch(F(&& u_Func)[N], const Utils::TaskOptions(&ra_Options)[N]) {
			return static_cast<Derived*>(this)->template callBatchC<F>(std::move(u_Func), std::move(ra_Options));
		}

		template<typename F>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> scheduleRepeatable(F&& u_Func, const Utils::TaskOptions& r_Options) {
			return static_cast<Derived*>(this)->template scheduleRepeatableC<F>(std::forward<F>(u_Func), r_Options);
		}


		[[nodiscard]] virtual size_t getScheduledTaskCount() const noexcept = 0;
		virtual bool cancelScheduled() const noexcept = 0;
		void delayedShutdown(auto&& u_Duration) {
			static_cast<Derived*>(this)->delayedShutdownC(std::forward<decltype(u_Duration)>(u_Duration));
		}
	};
}
