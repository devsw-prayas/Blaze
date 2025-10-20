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
	class CORIUM SpinPredicateCondition final : public ICondition {
		Platform::ParkHandle m_Permit;
		Locks::Lock& m_Lock;

	public:
		explicit SpinPredicateCondition(L& ro_Lock) noexcept : m_Lock(ro_Lock) {}

		template<typename F> requires Traits::IsPredicateV<F>
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
				std::atomic_thread_fence(std::memory_order_acquire);
				m_Lock.lock<L>();
			}
		}

		template<typename F> requires Traits::IsPredicateV<F>
		[[nodiscard]] bool tryAwaitC(F&& u_Predicate) noexcept {
			auto&& bound = std::forward<F>(u_Predicate);
			return bound();
		}

		template<typename F, typename T> requires Traits::IsDurationV<T>&& Traits::IsPredicateV<F>
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
				std::atomic_thread_fence(std::memory_order_acquire);
				m_Lock.lock<L>();
			}
			return bound();
		}

		template<typename F, typename T> requires Traits::IsTimePointV<T>&& Traits::IsPredicateV<F>
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
				std::atomic_thread_fence(std::memory_order_acquire);
				m_Lock.lock<L>();
			}
			return bound();
		}

		void signalC() {
			m_Permit.increment(std::memory_order_release);
			Platform::NativeThread::wakeOnAddress(m_Permit);
		}

		void signalAllC() {
			m_Permit.increment(std::memory_order_release);
			Platform::NativeThread::wakeAllOnAddress(m_Permit);
		}

		SpinPredicateCondition(const SpinPredicateCondition&) = delete;
		SpinPredicateCondition& operator=(const SpinPredicateCondition&) = delete;
		SpinPredicateCondition(SpinPredicateCondition&&) noexcept = delete;
		SpinPredicateCondition& operator=(SpinPredicateCondition&&) noexcept = delete;

		~SpinPredicateCondition() = default;
	};

	template<typename L> requires std::is_base_of_v<Locks::Lock, L>
	using PredicateCondition = SpinPredicateCondition<L, 0>;

	struct MCSNode final {
		MCSNode* m_NextNode;
		Platform::ParkHandle* m_Permit;
		std::atomic<bool> m_Waiting;

		MCSNode() : m_NextNode(nullptr), m_Permit(nullptr), m_Waiting(false) {}
	};

	namespace this_thread {
		inline thread_local MCSNode t_ThisNode{};
	}

	template<size_t SpinLimit = 400>
	class MCSCondition final : ICondition {
		std::atomic<MCSNode*> m_Head;
		std::atomic<MCSNode*> m_Tail;

	public:
		MCSCondition() :m_Head(nullptr), m_Tail(nullptr) {}

		void awaitC() noexcept {
			this_thread::t_ThisNode.m_NextNode = nullptr;
			this_thread::t_ThisNode.m_Waiting = true;
			this_thread::t_ThisNode.m_Permit = &Platform::this_platform_thread::t_ParkingPermit;

			if (auto* node = m_Tail.exchange(&this_thread::t_ThisNode, std::memory_order_acq_rel)) {
				node->m_NextNode = &this_thread::t_ThisNode;
			} else {
				m_Head = &this_thread::t_ThisNode;
				m_Tail = &this_thread::t_ThisNode;
			}
			while (this_thread::t_ThisNode.m_Waiting) {
				for (int i = 0; i < SpinLimit; i++) PAUSE

					std::atomic_signal_fence(std::memory_order_seq_cst);
				Platform::NativeThread::waitOnAddress(*this_thread::t_ThisNode.m_Permit);
			}

			if (auto* next = this_thread::t_ThisNode.m_NextNode) {
				m_Head.store(next, std::memory_order_release);
				next->m_Waiting = false;
				Platform::NativeThread::wakeOnAddress(*next->m_Permit);
			} else if (m_Tail == &this_thread::t_ThisNode) {
				MCSNode* node = &this_thread::t_ThisNode;
				if (!m_Tail.compare_exchange_strong(node, nullptr)) {
					// CAS Failed, wait to prevent race window and data overwrite
					while (!(next = this_thread::t_ThisNode.m_NextNode)) {
						PAUSE
					}
					m_Head.store(next, std::memory_order_release);
					std::atomic_thread_fence(std::memory_order_acq_rel);
					next->m_Waiting = false;
					Platform::NativeThread::wakeOnAddress(*next->m_Permit);
				} else m_Head.store(nullptr, std::memory_order_release);
			}
		}

		void signalC() const {
			if (auto* head = m_Head.load(std::memory_order_acquire)) {
				head->m_Waiting = false;
				Platform::NativeThread::wakeOnAddress(*head->m_Permit);
			}
		}
	};

	struct FIFONode final {
		std::atomic<FIFONode*> m_NextNode;
		Platform::ParkHandle* m_Permit;

		FIFONode() :m_NextNode(nullptr), m_Permit(nullptr) {}
	};

	namespace this_thread {
		inline FIFONode t_Node{};
	}

	template<typename L, size_t SpinLimit> requires std::is_base_of_v<Locks::Lock, L>
	class FIFOCondition final : ICondition {
		Locks::Lock& m_Lock;
		std::atomic<FIFONode*> m_Head;
		std::atomic<FIFONode*> m_Tail;

	public:
		FIFOCondition(L& ro_Lock) : m_Lock(ro_Lock) {}

		template<typename F> requires Traits::IsPredicateV<F>
		void awaitC(F&& u_Predicate) noexcept {
			this_thread::t_Node.m_Permit = &Platform::this_platform_thread::t_ParkingPermit;
			this_thread::t_Node.m_NextNode = nullptr;
			if (auto* node = m_Tail.exchange(&this_thread::t_Node, std::memory_order_acq_rel)) {
				node->m_NextNode.store(&this_thread::t_Node, std::memory_order_release);
			} else {
				m_Head.store(&this_thread::t_Node, std::memory_order_release);
			}

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
				Platform::NativeThread::waitOnAddress(*this_thread::t_Node.m_Permit);
				std::atomic_thread_fence(std::memory_order_acquire);
				m_Lock.lock<L>();
			}
			this_thread::t_Node.m_NextNode = nullptr;
			this_thread::t_Node.m_Permit = nullptr;
		}

		void signalC() {
			if (auto* head = m_Head.load(std::memory_order_acquire)) {
				FIFONode* next = head->m_NextNode.load(std::memory_order_acquire);
				m_Head.store(next, std::memory_order_release);
				if (!next) m_Tail.store(nullptr, std::memory_order_release);
				Platform::NativeThread::wakeOnAddress(*head->m_Permit);
			}
		}
	};

	struct alignas(64) EventShard final {
		Platform::ParkHandle m_Permit{};
		std::atomic<bool> m_Waiting{ false };
		std::atomic<size_t> m_Parked{ 0 };
		size_t m_Hash = Utils::hash();
	};

	template<size_t SpinLimit = 400, size_t ShardDensity = 128>
	class ShardedEventCondition final : ICondition {
		alignas(64) EventShard m_Shards[ShardDensity];
		std::atomic<size_t> m_ThreadShard{ 0 };

	public:
		size_t chooseShard() {
			return m_ThreadShard.fetch_add(1, std::memory_order_acq_rel) % ShardDensity;
		}

		void awaitC() noexcept {
			size_t shardIndex = chooseShard();

			EventShard& shard = m_Shards[shardIndex];
			bool wait = shard.m_Waiting.load(std::memory_order_acquire);
			shard.m_Parked.fetch_add(1, std::memory_order_acq_rel);
			while (wait) {
				for (size_t i = 0; i < SpinLimit; i++) {
					if (!shard.m_Waiting.load(std::memory_order_acquire)) {
						shard.m_Parked.fetch_sub(1, std::memory_order_relaxed);
						return;
					}
					PAUSE
				}
				Platform::NativeThread::waitOnAddress(shard.m_Permit);
			}
			shard.m_Parked.fetch_sub(1, std::memory_order_relaxed);
		}

		void await(size_t v_ShardHash) noexcept {
			EventShard& shard = m_Shards[v_ShardHash];
			bool wait = shard.m_Waiting.load(std::memory_order_acquire);
			shard.m_Parked.fetch_add(1, std::memory_order_acq_rel);
			while (wait) {
				for (size_t i = 0; i < SpinLimit; i++) {
					if (!shard.m_Waiting.load(std::memory_order_acquire)) {
						shard.m_Parked.fetch_sub(1, std::memory_order_relaxed);
						return;
					}
					PAUSE
				}
				Platform::NativeThread::waitOnAddress(shard.m_Permit);
			}
			shard.m_Parked.fetch_sub(1, std::memory_order_relaxed);
		}

		void signalC() {
			size_t maxLoad = m_Shards[0].m_Parked.load(std::memory_order_acquire), loadShard = 0;
			for (size_t i = 1; i < ShardDensity; i++) {
				size_t val = m_Shards[i].m_Parked.load(std::memory_order_acquire);
				if (maxLoad < val) {
					maxLoad = val;
					loadShard = i;
				}
			}

			EventShard& shard = m_Shards[loadShard];
			shard.m_Waiting.store(false, std::memory_order_release);
			Platform::NativeThread::wakeAllOnAddress(shard.m_Permit);
			std::atomic_thread_fence(std::memory_order_acq_rel);
		}

		void signal(size_t v_ShardHash) {
			EventShard& shard = m_Shards[v_ShardHash];
			shard.m_Waiting.store(false, std::memory_order_release);
			Platform::NativeThread::wakeAllOnAddress(shard.m_Permit);
			std::atomic_thread_fence(std::memory_order_acq_rel);
		}
	};

	template<size_t SpinLimit = 400>
	using ShardedEvent128 = ShardedEventCondition<SpinLimit>;

	template<size_t SpinLimit = 400>
	using ShardedEvent64 = ShardedEventCondition<SpinLimit, 64>;

	template<size_t SpinLimit = 400>
	using ShardedEvent32 = ShardedEventCondition<SpinLimit, 32>;
}