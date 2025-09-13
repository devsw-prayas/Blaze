#pragma once
#include "CoriumAtomics.h"
#include "CoriumConditions.h"
#include "CoriumLocks.h"
#include "ThreadPlatform.h"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#elif defined(__linux__)
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <time.h>
#include <sched.h>
#endif

namespace Corium::Sync::Conditions {
	template<typename L, size_t SpinLimit = 400> requires std::is_base_of_v<Locks::Lock, L>
	class SpinPredicateCondition final : public Condition {
		Platform::ParkHandle m_Permit;
		L& m_Lock;

	public:
		explicit SpinPredicateCondition(L& ro_Lock) noexcept : m_Lock(ro_Lock) {}

		template<typename F>
		void awaitC(F&& u_Predicate) noexcept {
			auto bound = std::forward<F>(u_Predicate);

			while (!bound()) {
				for (size_t i = 0; i < SpinLimit; i++) PAUSE
					
			}
		}

		template<typename F>
		bool tryAwaitC(F&& u_Predicate) noexcept {}

		template<typename F, typename T>
		bool tryAwaitForC(F&& u_Predicate, T&& u_Duration) noexcept {}

		template<typename F, typename T>
		bool tryAwaitUntilC(F&& u_Predicate, T&& u_Duration) noexcept {}

		void signalC() {}

		void signalAllC() {}

		SpinPredicateCondition(const SpinPredicateCondition&) = delete;
		SpinPredicateCondition& operator=(const SpinPredicateCondition&) = delete;
		SpinPredicateCondition(SpinPredicateCondition&&) noexcept = delete;
		SpinPredicateCondition& operator=(SpinPredicateCondition&&) noexcept = delete;

		~SpinPredicateCondition() = default;
	};

	template<typename L> requires std::is_base_of_v<Locks::Lock, L>
	class MCSPredicateCondition final : Condition {};
}