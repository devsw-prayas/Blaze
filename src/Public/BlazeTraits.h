#pragma once
#include "Blaze.h"
#include "BlazeMemory.h"
#include "BlazeUtils.h"

namespace Blaze::Traits {
	template<typename D, typename F, typename ...Args>
	concept HasVariadicSubmitC = requires(D * p_D, F && u_Func, const Utils::TaskOptions & r_Options, Args&&...u_Args) {
		{ p_D-> template submitC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...) }
		-> std::same_as<Memory::SharedPointer<Utils::IHandle>>;
	};

	template<typename D, typename F>
	concept HasSubmitC = requires(D * p_D, F && u_Func, const Utils::TaskOptions & r_Options) {
		{ p_D-> template submitC<F>(std::forward<F>(u_Func), r_Options) }
		->std::same_as<Memory::SharedPointer<Utils::IHandle>>;
	};

	template<typename D, typename F, typename ...Args>
	concept HasVariadicFutureC = requires(D * p_D, F && u_Func, const Utils::TaskOptions & r_Options, Args&&...u_Args) {
		{ p_D-> template futureC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...) }
		-> std::same_as<Memory::SharedPointer<Utils::IHandle>>;
	};

	template<typename D, typename F>
	concept HasFutureC = requires(D * p_D, F && u_Func, const Utils::TaskOptions & r_Options) {
		{ p_D-> template futureC<F>(std::forward<F>(u_Func), r_Options) }
		-> std::same_as<Memory::SharedPointer<Utils::IHandle>>;
	};

	template<typename D, typename F, typename ...Args>
	concept HasVariadicScheduleRunC = requires(D * p_D, F && u_Func, const Utils::TaskOptions & r_Options, auto&& u_Duration, Args&&...u_Args) {
		{ p_D-> template scheduleRunC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<decltype(u_Duration)>(u_Duration), std::forward<Args>(u_Args)...) }
		-> std::same_as<Memory::SharedPointer<Utils::IHandle>>;
	};

	template<typename D, typename F>
	concept HasScheduleRunC = requires(D * p_D, F && u_Func, const Utils::TaskOptions & r_Options, auto&& u_Duration) {
		{ p_D-> template scheduleRunC<F>(std::forward<F>(u_Func), r_Options, std::forward<decltype(u_Duration)>(u_Duration)) }
		->std::same_as<Memory::SharedPointer<Utils::IHandle>>;
	};

	template<typename D, typename F, typename ...Args>
	concept HasVariadicScheduleFutureC = requires(D * p_D, F && u_Func, const Utils::TaskOptions & r_Options, auto&& u_Duration, Args&&...u_Args) {
		{ p_D-> template scheduleFutureC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<decltype(u_Duration)>(u_Duration), std::forward<Args>(u_Args)...) }
		-> std::same_as<Memory::SharedPointer<Utils::IHandle>>;
	};

	template<typename D, typename F>
	concept HasScheduleFutureC = requires(D * p_D, F && u_Func, const Utils::TaskOptions & r_Options, auto&& u_Duration) {
		{ p_D-> template scheduleFutureC<F>(std::forward<F>(u_Func), r_Options, std::forward<decltype(u_Duration)>(u_Duration)) }
		-> std::same_as<Memory::SharedPointer<Utils::IHandle>>;
	};

	template<typename D, typename F, typename ...Args>
	concept HasVariadicRunC = requires(D * p_D, F && u_Func, const Utils::TaskOptions & r_Options, Args&&...u_Args) {
		{ p_D-> template runC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...) }
		-> std::same_as<void>;
	};

	template<typename D, typename F>
	concept HasRunC = requires(D * p_D, F && u_Func, const Utils::TaskOptions & r_Options) {
		{ p_D-> template runC<F>(std::forward<F>(u_Func), r_Options) }
		->std::same_as<void>;
	};

	template<typename D, typename F, typename ...Args>
	concept HasVariadicCallC = requires(D * p_D, F && u_Func, const Utils::TaskOptions & r_Options, Args&&...u_Args) {
		p_D-> template callC<F, Args...>(std::forward<F>(u_Func), r_Options, std::forward<Args>(u_Args)...);
	};

	template<typename D, typename F>
	concept HasCallC = requires(D * p_D, F && u_Func, const Utils::TaskOptions & r_Options) {
		p_D-> template callC<F>(std::forward<F>(u_Func), r_Options);
	};

	template<typename D, typename F, size_t N>
	concept HasSubmitBatchC = requires(D * p_D, F(&& u_Func)[N], const Utils::TaskOptions(&ra_Options)[N]) {
		{ p_D-> template submitBatchC<F>(std::move(u_Func), std::move(ra_Options)) }
		-> std::same_as<std::array<Memory::SharedPointer<Utils::IHandle>, N>>;
	};

	template<typename D, typename F, size_t N>
	concept HasFutureBatchC = requires(D * p_D, F(&& u_Func)[N], const Utils::TaskOptions(&ra_Options)[N]) {
		{ p_D-> template futureBatchC<F>(std::move(u_Func), std::move(ra_Options)) }
		-> std::same_as<std::array<Memory::SharedPointer<Utils::IHandle>, N>>;
	};

	template<typename D, typename F>
	concept HasScheduleRepeatableC = requires(D * p_D, F && u_Func, const Utils::TaskOptions & r_Options) {
		{ p_D-> template scheduleRepeatableC<F>(std::forward<F>(u_Func), r_Options) }
		->std::same_as<Memory::SharedPointer<Utils::IHandle>>;
	};

	template<typename D>
	concept hasForkC = requires(D * p_D, D && u_subTask) {
		{ p_D->forkC(std::forward<D>(u_subTask)) }
		-> std::same_as<Memory::SharedPointer<Utils::IHandle>>;
	};

	template<typename D>
	concept hasComputeC = requires(D * p_D) {
		p_D->computeC();
	};

	template<typename D>
	concept hasJoinC = requires(D * p_D, Memory::SharedPointer<Utils::IHandle> handle){
		p_D->joinC(handle);
	};
}
