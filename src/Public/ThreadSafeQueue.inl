#pragma once

namespace blaze {
	template<typename T>
	void ThreadSafeQueue<T>::push(T item) {
		{
			std::lock_guard lock(mutex_);
			queue_.push_back(std::move(item));
		}
		cond_.notify_one();	
	}

	template<typename T>
	template<typename... Args>
	void ThreadSafeQueue<T>::emplace(Args&&... args) {
		{
			std::lock_guard lock(mutex_);
			queue_.emplace_back(std::forward<Args>(args)...);
		}
		cond_.notify_one();
	}

	template<typename T>
	bool ThreadSafeQueue<T>::tryPop(T& item) {
		std::lock_guard lock(mutex_);
		if (queue_.empty()) return false;
		item = std::move(queue_.front());
		queue_.pop_front();
		return true;
	}

	template<typename T>
	std::optional<T> ThreadSafeQueue<T>::tryPop() {
		std::lock_guard lock(mutex_);
		if (queue_.empty()) return std::nullopt;
		T item = std::move(queue_.front());
		queue_.pop_front();
		return item;
	}

	template<typename T>
	bool ThreadSafeQueue<T>::trySteal(T& item) {
		std::lock_guard lock(mutex_);
		if (queue_.empty()) return false;
		item = std::move(queue_.back());
		queue_.pop_back();
		return true;
	}

	template<typename T>
	std::optional<T> ThreadSafeQueue<T>::trySteal() {
		std::lock_guard lock(mutex_);
		if (queue_.empty()) return std::nullopt;
		T item = std::move(queue_.back());
		queue_.pop_back();
		return item;
	}

	template<typename T>
	void ThreadSafeQueue<T>::waitAndPop(T& item) {
		std::unique_lock lock(mutex_);
		cond_.wait(lock, [this] { return !queue_.empty() || stop_waiting_; });
		if (stop_waiting_) {
			throw std::runtime_error("Queue wait interrupted");
		}
		item = std::move(queue_.front());
		queue_.pop_front();
	}

	template<typename T>
	std::optional<T> ThreadSafeQueue<T>::waitAndPop() {
		std::unique_lock lock(mutex_);
		cond_.wait(lock, [this] { return !queue_.empty() || stop_waiting_; });
		if (stop_waiting_) {
			return std::nullopt;
		}
		T item = std::move(queue_.front());
		queue_.pop_front();
		return item;
	}

	template<typename T>
	template<typename Rep, typename Period>
	bool ThreadSafeQueue<T>::waitAndPopFor(T& item, const std::chrono::duration<Rep, Period>& timeout) {
		std::unique_lock lock(mutex_);
		if (!cond_.wait_for(lock, timeout, [this] { return !queue_.empty() || stop_waiting_; })) {
			return false;
		}
		if (stop_waiting_) {
			throw std::runtime_error("Queue wait interrupted");
		}
		item = std::move(queue_.front());
		queue_.pop_front();
		return true;
	}

	template<typename T>
	template<typename Rep, typename Period>
	std::optional<T> ThreadSafeQueue<T>::waitAndPopFor(const std::chrono::duration<Rep, Period>& timeout) {
		std::unique_lock lock(mutex_);
		if (!cond_.wait_for(lock, timeout, [this] { return !queue_.empty() || stop_waiting_; })) {
			return std::nullopt;
		}
		if (stop_waiting_) {
			return std::nullopt;
		}
		T item = std::move(queue_.front());
		queue_.pop_front();
		return item;
	}

	template<typename T>
	void ThreadSafeQueue<T>::clear() {
		std::lock_guard lock(mutex_);
		queue_.clear();
	}

	template<typename T>
	size_t ThreadSafeQueue<T>::size() const {
		std::lock_guard lock(mutex_);
		return queue_.size();
	}

	template<typename T>
	bool ThreadSafeQueue<T>::empty() const {
		std::lock_guard lock(mutex_);
		return queue_.empty();
	}

	template<typename T>
	void ThreadSafeQueue<T>::stopWaiting() {
		{
			std::lock_guard lock(mutex_);
			stop_waiting_ = true;
		}
		cond_.notify_all();
	}

	template<typename T>
	void ThreadSafeQueue<T>::resetWaiting() {
		std::lock_guard lock(mutex_);
		stop_waiting_ = false;
	}

	template<typename T>
	std::optional<T> ThreadSafeQueue<T>::front() const {
		std::lock_guard<std::mutex> lock(mutex_);
		if (queue_.empty()) {
			return std::nullopt;
		}
		return queue_.front(); // Returns a copy of the front element
	}

	template<typename T>
	std::optional<T> ThreadSafeQueue<T>::back() const {
		std::lock_guard<std::mutex> lock(mutex_);
		if (queue_.empty()) {
			return std::nullopt;
		}
		return queue_.back(); // Returns a copy of the back element
	}
}