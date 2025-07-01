#pragma once

namespace blaze {
    // Insertions
    template<typename T>
    void ThreadSafePriorityQueue<T>::push(const T& item) {
        std::lock_guard lock(mutex_);
        queue_.push(item);
        cv_.notify_one();
    }

    template<typename T>
    void ThreadSafePriorityQueue<T>::push(T&& item) {
        std::lock_guard lock(mutex_);
        queue_.push(std::move(item));
        cv_.notify_one();
    }

    template<typename T>
    template<typename... Args>
    void ThreadSafePriorityQueue<T>::emplace(Args&&... args) {
        std::lock_guard lock(mutex_);
        queue_.emplace(std::forward<Args>(args)...);
        cv_.notify_one();
    }

    // Removals
    template<typename T>
    std::optional<T> ThreadSafePriorityQueue<T>::tryPop() {
        std::lock_guard lock(mutex_);
        if (queue_.empty()) return std::nullopt;
        T item = std::move(const_cast<T&>(queue_.top())); // Cast away const, then move
        queue_.pop();
        return item;
    }

    template<typename T>
    std::optional<T> ThreadSafePriorityQueue<T>::waitAndPop() {
        std::unique_lock lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty() || stop_notify_; });
        if (stop_notify_) return std::nullopt;
        T item = std::move(queue_.top());
        queue_.pop();
        return item;
    }

    template<typename T>
    std::optional<T> ThreadSafePriorityQueue<T>::waitAndPopFor(auto&& duration) {
        std::unique_lock lock(mutex_);
        if (!cv_.wait_for(lock, duration, [this] { return !queue_.empty() || stop_notify_; })) {
            return std::nullopt;
        }
        if (stop_notify_) return std::nullopt;
        T item = std::move(queue_.top());
        queue_.pop();
        return item;
    }

    // Management
    template<typename T>
    void ThreadSafePriorityQueue<T>::clear() {
        std::lock_guard lock(mutex_);
        while (!queue_.empty()) queue_.pop();
    }

    template<typename T>
    size_t ThreadSafePriorityQueue<T>::size() const {
        std::lock_guard lock(mutex_);
        return queue_.size();
    }

    template<typename T>
    bool ThreadSafePriorityQueue<T>::empty() const {
        std::lock_guard lock(mutex_);
        return queue_.empty();
    }

    template<typename T>
    void ThreadSafePriorityQueue<T>::notifyAll() {
        stop_notify_ = true;
        cv_.notify_all();
    }
}
