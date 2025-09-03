#pragma once
#include "CoriumTraits.h"

namespace Corium::Sync::Locks {

	class Lock {
	public:
		Lock(const Lock&) = delete;
		Lock(Lock&&) noexcept = delete;
		Lock& operator=(const Lock&) = delete;
		Lock& operator=(Lock&&) noexcept = delete;

		template<typename D>
		void lock() noexcept requires std::is_base_of_v<Lock, D> {
			static_cast<D*>(this)->lockC();
		}

		template<typename D>
		bool tryLock() noexcept requires std::is_base_of_v<Lock, D> {
			return static_cast<D*>(this)->tryLockC();
		}

		template<typename D>
		void unlock() noexcept requires std::is_base_of_v<Lock, D> {
			static_cast<D*>(this)->unlockC();
		}

		template<typename T, typename  D>
		bool tryLockFor(T&& u_Duration) noexcept requires std::conjunction_v<Traits::IsDuration<T>, std::is_base_of<Lock, D>> {
			return static_cast<D*>(this)->tryLockForC(std::forward<T>(u_Duration));
		}

		template<typename T, typename D>
		bool tryLockUntil(T&& u_TimePoint) noexcept requires std::conjunction_v<Traits::IsTimePoint<T>, std::is_base_of<Lock, D>> {
			return static_cast<D*>(this)->tryLockUntilC(std::forward<T>(u_TimePoint));
		}

	protected:
		~Lock() = default;
	};
}
