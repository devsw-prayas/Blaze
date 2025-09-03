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
		void await() noexcept requires std::is_base_of_v<Condition, D> {
			static_cast<D*>(this)->awaitC();
		}

		template<typename D>
		bool tryAwait() noexcept requires std::is_base_of_v<Condition, D> {
			return static_cast<D*>(this)->tryAwaitC();
		}

		template<typename T, typename D>
		bool tryAwaitFor(T&& u_Duration) noexcept requires std::conjunction_v<Traits::IsDuration<T>, std::is_base_of<Condition, D>> {
			return static_cast<D*>(this)->tryAwaitForC(std::forward<T>(u_Duration));
		}

		template<typename T, typename D>
		bool tryAwaitUntil(T&& u_TimePoint) noexcept requires std::conjunction_v<Traits::IsTimePoint<T>, std::is_base_of<Condition, D>> {
			return static_cast<D*>(this)->tryAwaitUntilC(std::forward<T>(u_TimePoint));
		}

		template<typename D>
		void signal() {
			static_cast<D*>(this)->signalC();
		}
	protected:
		~Condition() = default;
	};
}
