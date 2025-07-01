#pragma once

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <atomic>
#include <stdexcept>

namespace blaze{
	template<typename T>
	class Blaze ThreadSafePriorityQueue {
	public:
		ThreadSafePriorityQueue() = default;
		~ThreadSafePriorityQueue() = default;

		// Delete copy/move operations
		ThreadSafePriorityQueue(const ThreadSafePriorityQueue&) = delete;
		ThreadSafePriorityQueue& operator=(const ThreadSafePriorityQueue&) = delete;
		ThreadSafePriorityQueue(ThreadSafePriorityQueue&&) = delete;
		ThreadSafePriorityQueue& operator=(ThreadSafePriorityQueue&&) = delete;

		// Core interface
		void push(const T& item);
		void push(T&& item);
		template<typename... Args> void emplace(Args&&... args);

		std::optional<T> tryPop();
		std::optional<T> waitAndPop();
		std::optional<T> waitAndPopFor(auto&& duration);

		void clear();
		size_t size() const;
		bool empty() const;

		void notifyAll(); // For shutdown

	private:
		mutable std::mutex mutex_;
		std::condition_variable cv_;
		std::priority_queue<T, std::vector<T>, std::less<T>> queue_; // Max-heap by default
		std::atomic<bool> stop_notify_{ false };
	};
}

#include "ThreadSafePriorityQueue.inl"