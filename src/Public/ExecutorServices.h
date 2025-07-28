#pragma once
#include "Blaze.h"
#include "BlazeEvent.h"
#include "Executors.h"
namespace Blaze::Executors::Services {
	struct EventContractOnSubmit final{};
	struct EventContractOnCall final {};
	struct EventContractOnRun final {};

	template<typename Derived
#if ENABLE_EVENT_EMITTERS_BLAZE
	, typename H = Events::EventEmitterPack<>::EventPack, size_t Hash = Events::DEFAULT_HASH
#endif
	>
	class BLAZE IThreadExecutorService : IExecutor<Derived>, IExecutorVirtual{
	public:
		static_assert(std::is_base_of_v<IThreadExecutorService, Derived>,
			"Derived is not a subclass of IThreadExecutorService");

		template<typename F, typename ...Args>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> submit(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_args) {
#if ENABLE_EVENT_EMITTERS_BLAZE

#endif
			return static_cast<Derived*>(this)->template submtiC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_args)...);
		}

		template<typename F>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> submit(F&& u_Func, const Utils::TaskOptions& r_Options) {
			return static_cast<Derived*>(this)->template submitC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename...Args>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> call(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_args) {
			return static_cast<Derived*>(this)->template callC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_args)...);
		}

		template<typename F>
		[[nodiscard]] Memory::SharedPointer<Utils::IHandle> call(F&& u_Func, const Utils::TaskOptions& r_Options) {
			return static_cast<Derived*>(this)->template callC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, typename...Args>
		void run(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
			static_cast<Derived*>(this)->template runC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		} 

		template<typename F>
		void run(F&& u_Func, const Utils::TaskOptions& r_Options) {
			static_cast<Derived*>(this)->template runC<F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename T, typename F, typename...Args>
		[[nodiscard]] T&& call(F&& u_Func, const Utils::TaskOptions& r_Options, Args&&... u_Args) {
			return static_cast<Derived*>(this)->template callC<T, F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
		}

		template<typename T, typename F>
		[[nodiscard]] T&& call(F&& u_Func, const Utils::TaskOptions& r_Options) {
			return static_cast<Derived*>(this)->template callC<T, F>(std::forward<F>(u_Func), r_Options);
		}

		template<typename F, size_t N>
		std::tuple<Memory::SharedPointer<Utils::IHandle>>
		submitBatch(F (&&u_Func)[N], const Utils::TaskOptions (&ra_Options)[N]) {
			return static_cast<Derived*>(this)->template submitBatchC<F>(std::move(u_Func), std::move(ra_Options));
		}

		template<typename F, size_t N>
		std::tuple<Memory::SharedPointer<Utils::IHandle>>
		callBatch(F (&&u_Func)[N], const Utils::TaskOptions (&ra_Options)[N]) {
			return static_cast<Derived*>(this)->template callBatchC<F>(std::move(u_Func), std::move(ra_Options));
		}
	};


}
