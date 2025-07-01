#pragma once
#include <deque>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <atomic>
#include <stdexcept>

namespace blaze {
    template<typename T>
    class Blaze ThreadSafeQueue {
    public:
        ThreadSafeQueue() = default;
        ~ThreadSafeQueue() = default;

        // Non-copyable and non-movable
        ThreadSafeQueue(const ThreadSafeQueue&) = delete;
        ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;
        ThreadSafeQueue(ThreadSafeQueue&&) = delete;
        ThreadSafeQueue& operator=(ThreadSafeQueue&&) = delete;

        // Element insertion
        void push(T item);
        template<typename... Args> void emplace(Args&&... args);

        // Element removal
        bool tryPop(T& item);
        std::optional<T> tryPop();
        bool trySteal(T& item);
        std::optional<T> trySteal();
        void waitAndPop(T& item);
        std::optional<T> waitAndPop();
        std::optional<T> front() const;
        std::optional<T> back() const;

        // Timed operations
        template<typename Rep, typename Period>
        bool waitAndPopFor(T& item, const std::chrono::duration<Rep, Period>& timeout);

        template<typename Rep, typename Period>
        std::optional<T> waitAndPopFor(const std::chrono::duration<Rep, Period>& timeout);

        // Queue management
        void clear();
        size_t size() const;
        bool empty() const;

        // Waiting control
        void stopWaiting();
        void resetWaiting();


    private:
        mutable std::mutex mutex_;
        std::condition_variable cond_;
        std::deque<T> queue_;
        std::atomic<bool> stop_waiting_{ false };
    };
}

#include "ThreadSafeQueue.inl"