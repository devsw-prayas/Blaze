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
#include "CoriumAtomics.h"
#include "CoriumConditions.h"
#include "CoriumLocks.h"
#include "ThreadPlatform.h"

namespace Corium::Sync::Conditions {
	template<typename L, size_t SpinLimit = 400> requires std::is_base_of_v<Locks::Lock, L>
	class CORIUM SpinPredicateCondition final : public Condition {
		Platform::ParkHandle m_Permit;
		Locks::Lock& m_Lock;

	public:
		explicit SpinPredicateCondition(L& ro_Lock) noexcept : m_Lock(ro_Lock) {}

		template<typename F>
		void awaitC(F&& u_Predicate) noexcept {
			auto&& bound = std::forward<F>(u_Predicate);
			while (!bound()) {
				m_Lock.unlock<L>();
				for (size_t i = 0; i < SpinLimit; i++) {
					PAUSE
						if (bound()) {
							m_Lock.lock<L>();
							return;
						}
				}
				Platform::NativeThread::waitOnAddress(m_Permit);
				m_Lock.lock<L>();
			}
		}

		template<typename F>
		[[nodiscard]] bool tryAwaitC(F&& u_Predicate) noexcept {
			auto&& bound = std::forward<F>(u_Predicate);
			return bound();
		}

		template<typename F, typename T> requires Traits::IsDurationV<T>
		[[nodiscard]] bool tryAwaitForC(F&& u_Predicate, T&& u_Duration) noexcept {
			auto deadline = std::chrono::steady_clock::now() + std::forward<T>(u_Duration);
			auto&& bound = std::forward<F>(u_Predicate);
			while (true) {
				if (bound()) return true;
				m_Lock.unlock<L>();
				auto now = std::chrono::steady_clock::now();
				if (now >= deadline) break;
				for (size_t i = 0; i < SpinLimit; i++) {
					PAUSE
						if (bound()) {
							m_Lock.lock<L>();
							return true;
						}
				}
				Platform::NativeThread::waitOnAddressFor(m_Permit, deadline - now);
				m_Lock.lock<L>();
			}
			return bound();
		}

		template<typename F, typename T> requires Traits::IsTimePointV<T>
		bool tryAwaitUntilC(F&& u_Predicate, T&& u_TimePoint) noexcept {
			auto&& bound = std::forward<F>(u_Predicate);
			auto deadline = static_cast<std::chrono::steady_clock::time_point>(std::forward<T>(u_TimePoint));
			while (true) {
				if (bound()) return true;
				m_Lock.unlock<L>();
				auto now = std::chrono::steady_clock::now();
				if (now >= deadline) break;
				for (size_t i = 0; i < SpinLimit; i++) {
					PAUSE
						if (bound()) {
							m_Lock.lock<L>();
							return true;
						}
				}
				Platform::NativeThread::waitOnAddressFor(m_Permit, deadline - now);
				m_Lock.lock<L>();
			}
			return bound();
		}

		void signalC() {
			m_Permit.increment(std::memory_order_seq_cst);
			Platform::NativeThread::wakeOnAddress(m_Permit);
		}

		void signalAllC() {
			m_Permit.increment(std::memory_order_seq_cst);
			Platform::NativeThread::wakeAllOnAddress(m_Permit);
		}

		SpinPredicateCondition(const SpinPredicateCondition&) = delete;
		SpinPredicateCondition& operator=(const SpinPredicateCondition&) = delete;
		SpinPredicateCondition(SpinPredicateCondition&&) noexcept = delete;
		SpinPredicateCondition& operator=(SpinPredicateCondition&&) noexcept = delete;

		~SpinPredicateCondition() = default;
	};

	struct MCSNode final {
		MCSNode* m_NextNode;
		Platform::ParkHandle* m_Permit;
		std::atomic<bool> m_Waiting;
	};

	namespace this_thread {
		inline thread_local MCSNode t_ThisNode{};
	}

	template<size_t SpinLimit = 400>
	class MCSCondition final : Condition {
		std::atomic<MCSNode*> m_Head;
		std::atomic<MCSNode*> m_Tail;

	public:
		MCSCondition() :m_Head(nullptr), m_Tail(nullptr) {}

		void awaitC() noexcept {
			this_thread::t_ThisNode.m_NextNode = nullptr;
			this_thread::t_ThisNode.m_Waiting = true;
			this_thread::t_ThisNode.m_Permit = &Platform::this_platform_thread::t_ParkingPermit;

			if (auto* node = m_Tail.exchange(&this_thread::t_ThisNode)) {
				node->m_NextNode = &this_thread::t_ThisNode;
			} else {
				m_Head = &this_thread::t_ThisNode;
				m_Tail = &this_thread::t_ThisNode;
			}
			while (this_thread::t_ThisNode.m_Waiting) {
				for (int i = 0; i < SpinLimit; i++) PAUSE

					Platform::NativeThread::waitOnAddress(*this_thread::t_ThisNode.m_Permit);
			}

			if (auto* next = this_thread::t_ThisNode.m_NextNode) {
				m_Head.store(next, std::memory_order_seq_cst);
				next->m_Waiting = false;
				Platform::NativeThread::wakeOnAddress(*next->m_Permit);
			} else if (m_Tail == &this_thread::t_ThisNode) {
				MCSNode* node = &this_thread::t_ThisNode;
				if (!m_Tail.compare_exchange_strong(node, nullptr)) {
					while (!(next = this_thread::t_ThisNode.m_NextNode)) {
						PAUSE
					}
					m_Head.store(next, std::memory_order_seq_cst);
					next->m_Waiting = false;
					Platform::NativeThread::wakeOnAddress(*next->m_Permit);
				} else m_Head.store(nullptr, std::memory_order_seq_cst);
			}
		}

		void signalC() const {
			if (auto* head = m_Head.load(std::memory_order_acquire)) {
				head->m_Waiting = false;
				Platform::NativeThread::wakeOnAddress(*head->m_Permit);
			}
		}
	};

	template<typename L, size_t SpinLimit = 400> requires std::is_base_of_v<Locks::Lock, L>
	class SpinMCSHybridCondition final : Condition{/*TODO Hybrid */};
}