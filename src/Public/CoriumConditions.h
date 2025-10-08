#pragma once
#include "CoriumTraits.h"

namespace Corium::Sync::Conditions {
	class Condition {
	public:
		Condition(const Condition&) = delete;
		Condition(Condition&&) noexcept = delete;

		Condition& operator=(const Condition&) = delete;
		Condition& operator=(Condition&&) noexcept = delete;

		template<typename D>
		void await() noexcept
			requires std::is_base_of_v<Condition, D> {
			static_cast<D*>(this)->awaitC();
		}

		template<typename D, typename F>
		void await(F&& u_Predicate) noexcept
		requires std::conjunction_v<std::is_base_of<Condition, D>, Traits::IsPredicate<F>>{
			static_cast<D*>(this)->awaitC(std::forward<F>(u_Predicate));
		}

		template<typename D, typename F>
		bool tryAwait(F&& u_Predicate) noexcept 
		requires std::conjunction_v<std::is_base_of<Condition, D>, Traits::IsPredicate<F>> {
			return static_cast<D*>(this)->tryAwaitC(std::forward<F>(u_Predicate));
		}

		template<typename D, typename T, typename F>
		bool tryAwaitFor(F&& u_Predicate, T&& u_Duration) noexcept
		requires std::conjunction_v<Traits::IsDuration<T>, Traits::IsPredicate<F>, std::is_base_of<Condition, D>> {
			return static_cast<D*>(this)->tryAwaitForC(std::forward<F>(u_Predicate), std::forward<T>(u_Duration));
		}

		template<typename D, typename T, typename F>
		bool tryAwaitUntil(F&& u_Predicate, T&& u_TimePoint) noexcept
		requires std::conjunction_v<Traits::IsTimePoint<T>,  Traits::IsPredicate<F>,std::is_base_of<Condition, D>> {
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
		Condition() = default;	
		~Condition() = default;
	};
}
