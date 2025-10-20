#pragma once
#include "CoriumTraits.h"

namespace Corium::Sync::Conditions {
	class ICondition {
	public:
		ICondition(const ICondition&) = delete;
		ICondition(ICondition&&) noexcept = delete;

		ICondition& operator=(const ICondition&) = delete;
		ICondition& operator=(ICondition&&) noexcept = delete;

		template<typename D>
		void await() noexcept
			requires std::is_base_of_v<ICondition, D> {
			static_cast<D*>(this)->awaitC();
		}

		template<typename D, typename F>
		void await(F&& u_Predicate) noexcept
		requires std::conjunction_v<std::is_base_of<ICondition, D>, Traits::IsPredicate<F>>{
			static_cast<D*>(this)->awaitC(std::forward<F>(u_Predicate));
		}

		template<typename D, typename F>
		bool tryAwait(F&& u_Predicate) noexcept 
		requires std::conjunction_v<std::is_base_of<ICondition, D>, Traits::IsPredicate<F>> {
			return static_cast<D*>(this)->tryAwaitC(std::forward<F>(u_Predicate));
		}

		template<typename D, typename T, typename F>
		bool tryAwaitFor(F&& u_Predicate, T&& u_Duration) noexcept
		requires std::conjunction_v<Traits::IsDuration<T>, Traits::IsPredicate<F>, std::is_base_of<ICondition, D>> {
			return static_cast<D*>(this)->tryAwaitForC(std::forward<F>(u_Predicate), std::forward<T>(u_Duration));
		}

		template<typename D, typename T, typename F>
		bool tryAwaitUntil(F&& u_Predicate, T&& u_TimePoint) noexcept
		requires std::conjunction_v<Traits::IsTimePoint<T>,  Traits::IsPredicate<F>,std::is_base_of<ICondition, D>> {
			return static_cast<D*>(this)->tryAwaitUntilC(std::forward<F>(u_Predicate), std::forward<T>(u_TimePoint));
		}

		template<typename D>
		void signal() {
			static_cast<D*>(this)->signalC();
		}

		template<typename D>
		void signalAll() {
			static_cast<D*>(this)->signalAllC();
		}
	protected:
		ICondition() = default;	
		~ICondition() = default;
	};
}
