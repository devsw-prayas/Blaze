#include "ThreadPool.h"

#include <complex>
#include <numeric>

namespace blaze {
	// =============================================
	// Default Thread Pool Implementation
	// =============================================
	DefaultThreadPool::DefaultThreadPool(size_t numThreads, CPUThreadFactory& factory) {
		if (numThreads <= 0) {
			//TODO...
		}

		const size_t numCores = handles::CPUThreadHandle::getNumCores();
		const size_t numNumaNodes = handles::CPUThreadHandle::getNumaNodeCount();
		workers_.reserve(numThreads);
		threads_.reserve(numThreads);
		for (size_t i = 0; i < numThreads; i++) {
			workers_.push_back(std::make_unique<Worker>(static_cast<int>(i % numCores),
				numNumaNodes == 1 ? -1 : static_cast<int>(i % numNumaNodes)));
			workers_[i]->handle.workerId = i;
			handles::CPUThreadOptions options;
			options.name = "DefaultThreadPoolWorker-" + std::to_string(i);
			options.coreID = workers_[i]->handle.coreId;
			options.numaNode = workers_[i]->handle.numaNode;

			threads_.emplace_back(i, factory.createThread(options, [this, i] {
				workerFunction(i); }, false));
		}
		buildNumaWorkerMap();
	}

	void DefaultThreadPool::buildNumaWorkerMap() {
		std::unique_lock<std::shared_mutex> locker(numaMapMutex_);
		if (handles::CPUThreadHandle::getNumaNodeCount() <= 1) {
			numaToWorkers_[-1].resize(workers_.size());
			std::iota(numaToWorkers_[-1].begin(), numaToWorkers_[-1].end(), 0); // Fill with 0, 1, 2, ...
		}
		else {
			for (size_t i = 0; i < workers_.size(); i++) {
				numaToWorkers_[workers_[i]->handle.numaNode].push_back(i);
			}
		}
	}

	TaskID DefaultThreadPool::generateTaskId() noexcept {
		return taskIdCounter_.fetch_add(1, std::memory_order_relaxed);
	}

	size_t DefaultThreadPool::selectWorkerByNumaNode(int numaNode) const {
		std::shared_lock<std::shared_mutex> locker(numaMapMutex_);
		auto it = numaToWorkers_.find(numaNode);
		if (it == numaToWorkers_.end()) {
			return findLeastBusyWorker();
		}
		return it->second[0];
	}

	DefaultThreadPool::~DefaultThreadPool() {
		shutdownNow();
	}

	size_t DefaultThreadPool::findLeastBusyWorker() const {
		if (workers_.empty()) {
			return 0;
		}

		const size_t n = workers_.size();
		const size_t k = std::max(static_cast<size_t>(std::sqrt(static_cast<double>(n))),
			static_cast<size_t>(1UL));
		const size_t stride = n / k;

		size_t minTasks = std::numeric_limits<size_t>::max();
		size_t bestIndex = 0;

		for (size_t i = 0, steps = 0; steps < k && i < n; i += stride, ++steps) {
			size_t tasks = workers_[i]->handle.activeTasks.load(std::memory_order_relaxed);
			if (tasks < minTasks) {
				minTasks = tasks;
				bestIndex = i;
			}
		}

		return bestIndex;
	}

	bool DefaultThreadPool::isRunning() const noexcept {
		return isRunning_;
	}

	bool DefaultThreadPool::isShutdown() const noexcept {
		return isShutdown_;
	}

	bool DefaultThreadPool::isTerminated() const noexcept {
		return isTerminated_;
	}

	TaskState DefaultThreadPool::getTaskState(IHandle& handle) const {
		if (handle.getId() == static_cast<size_t>(-1)) {
			return TaskState::Unknown;
		}
		for (const auto& worker : workers_) {
			std::lock_guard<std::mutex> locker(worker->mutex);
			auto it = worker->taskStates.find(handle.getId());
			if (it != worker->taskStates.end()) {
				auto state = it->second.lock();
				return state ? *state : TaskState::Unknown;
			}
		}
		return TaskState::Unknown;
	}

	size_t DefaultThreadPool::getActiveTaskCount() const noexcept {
		return std::accumulate(
			std::ranges::begin(workers_), std::ranges::end(workers_), size_t{ 0 },
			[](size_t sum, const auto& worker) {
				return sum + worker->handle.activeTasks.load(std::memory_order_acquire);
			});
	}

	size_t DefaultThreadPool::getCompletedTaskCount() const noexcept {
		return std::accumulate(
			std::ranges::begin(workers_), std::ranges::end(workers_), size_t{ 0 },
			[](size_t sum, const auto& worker) {
				return sum + worker->handle.completedTasks.load(std::memory_order_acquire);
			});
	}

	bool DefaultThreadPool::cancelAllPending() const {
		return false; //TODO
	}

	std::shared_ptr<IHandle> DefaultThreadPool::submit(std::function<void()> task, const TaskOptions& options) {
		if (!isRunning() || isShutdown()) {
			return std::make_shared<ActionHandle>(static_cast<size_t>(-1), true);
		}
		auto handle = std::make_shared<ActionHandle>(generateTaskId(), false);
		size_t workerIndex;

		if (options.numaNodeAffinity >= 0) {
			workerIndex = selectWorkerByNumaNode(options.numaNodeAffinity);
		}
		else {
			workerIndex = findLeastBusyWorker();
		}
		{
			std::lock_guard<std::mutex> locker(workers_[workerIndex]->mutex);
			workers_[workerIndex]->queue.push(TaskEntry(handle, std::move(task), options.priority));
			*workers_[workerIndex]->taskStates[handle->getId()].lock() = TaskState::Pending;
		}

		workers_[workerIndex]->cv.notify_one();
		return handle;
	}

	std::vector<std::shared_ptr<IHandle>> DefaultThreadPool::submitBatch(std::vector<std::function<void()>> tasks, const std::vector<TaskOptions>& options) {
		if (!isRunning() || isShutdown()) {
			std::vector<std::shared_ptr<IHandle>> handles;
			handles.reserve(tasks.size());
			for (size_t i = 0; i < tasks.size(); ++i) {
				handles.emplace_back(std::make_shared<ActionHandle>(static_cast<uint64_t>(-1), true));
			}
			return handles;
		}

		// Bulk ID generation
		std::vector<std::shared_ptr<IHandle>> handles;
		handles.reserve(tasks.size());
		uint64_t baseId = taskIdCounter_.fetch_add(tasks.size(), std::memory_order_relaxed);
		for (size_t i = 0; i < tasks.size(); ++i) {
			handles.emplace_back(std::make_shared<ActionHandle>(baseId + i, false));
		}
		size_t size = tasks.size();
		// Scatter worker picks
		std::vector<size_t> workerIndices(tasks.size());
		if (handles::CPUThreadHandle::getNumaNodeCount() > 1) { // Assuming this is your NUMA check
			std::shared_lock<std::shared_mutex> lock(numaMapMutex_);
			for (size_t i = 0; i < tasks.size(); ++i) {
				int affinity = options[i].numaNodeAffinity;
				if (affinity >= 0 && !numaToWorkers_[affinity].empty()) {
					workerIndices[i] = numaToWorkers_[affinity][i % numaToWorkers_[affinity].size()];
				}
				else {
					workerIndices[i] = i % workers_.size(); // Round-robin fallback
				}
			}
		}
		else {
			for (size_t i = 0; i < tasks.size(); ++i) {
				workerIndices[i] = i % workers_.size(); // Simple round-robin
			}
		}

		// Queue tasks with per-worker locks
		std::vector<bool> notified(workers_.size(), false);
		for (size_t i = 0; i < size; ++i) {
			size_t idx = workerIndices[i];
			std::lock_guard<std::mutex> locker(workers_[idx]->mutex);
			workers_[idx]->queue.push(TaskEntry(handles[i], std::move(tasks[i]), options[i].priority));
			*workers_[idx]->taskStates[handles[i]->getId()].lock() = TaskState::Pending;
			if (!notified[idx]) {
				workers_[idx]->cv.notify_one();
				notified[idx] = true;
			}
		}

		return handles;
	}

	std::shared_ptr<IHandle> DefaultThreadPool::submitCallable(std::function<std::any()> task, const TaskOptions& options) {
		if (!isRunning() || isShutdown()) {
			return std::make_shared<ActionHandle>(static_cast<size_t>(-1), true);
		}
		auto handle = std::make_shared<TaskHandle>(generateTaskId(), false, nullptr);
		size_t workerIndex;
		if (options.numaNodeAffinity >= 0) {
			workerIndex = selectWorkerByNumaNode(options.numaNodeAffinity);
		}
		else {
			workerIndex = findLeastBusyWorker();
		}
		{
			std::lock_guard<std::mutex> locker(workers_[workerIndex]->mutex);
			workers_[workerIndex]->queue.push(TaskEntry(handle, std::move(task), options.priority));
			*workers_[workerIndex]->taskStates[handle->getId()].lock() = TaskState::Pending;
		}

		workers_[workerIndex]->cv.notify_one();
		return handle;
	}

	std::vector<std::shared_ptr<IHandle>> DefaultThreadPool::submitBatchCallable(std::vector<std::function<std::any()>> tasks, std::vector<TaskOptions>& options) {
		if (!isRunning() || isShutdown()) {
			std::vector<std::shared_ptr<IHandle>> handles;
			handles.reserve(tasks.size());
			for (size_t i = 0; i < tasks.size(); ++i) {
				handles.emplace_back(std::make_shared<TaskHandle>(static_cast<uint64_t>(-1), true, nullptr));
			}
			return handles;
		}

		// Bulk ID generation
		std::vector<std::shared_ptr<IHandle>> handles;
		handles.reserve(tasks.size());
		uint64_t baseId = taskIdCounter_.fetch_add(tasks.size(), std::memory_order_relaxed);
		for (size_t i = 0; i < tasks.size(); ++i) {
			handles.emplace_back(std::make_shared<TaskHandle>(baseId + i, false, nullptr));
		}
		size_t size = tasks.size();
		// Scatter worker picks
		std::vector<size_t> workerIndices(tasks.size());
		if (handles::CPUThreadHandle::getNumaNodeCount() > 1) { // Assuming this is your NUMA check
			std::shared_lock<std::shared_mutex> lock(numaMapMutex_);
			for (size_t i = 0; i < tasks.size(); ++i) {
				int affinity = options[i].numaNodeAffinity;
				if (affinity >= 0 && !numaToWorkers_[affinity].empty()) {
					workerIndices[i] = numaToWorkers_[affinity][i % numaToWorkers_[affinity].size()];
				}
				else {
					workerIndices[i] = i % workers_.size(); // Round-robin fallback
				}
			}
		}
		else {
			for (size_t i = 0; i < tasks.size(); ++i) {
				workerIndices[i] = i % workers_.size(); // Simple round-robin
			}
		}

		// Queue tasks with per-worker locks
		std::vector<bool> notified(workers_.size(), false);
		for (size_t i = 0; i < size; ++i) {
			size_t idx = workerIndices[i];
			std::lock_guard<std::mutex> locker(workers_[idx]->mutex);
			workers_[idx]->queue.push(TaskEntry(handles[i], std::move(tasks[i]), options[i].priority));
			*workers_[idx]->taskStates[handles[i]->getId()].lock() = TaskState::Pending;
			if (!notified[idx]) {
				workers_[idx]->cv.notify_one();
				notified[idx] = true;
			}
		}
		return handles;
	}

	void DefaultThreadPool::execute(std::function<void()> task) {
		submit(task);
	}

	void DefaultThreadPool::workerFunction(size_t workerIndex) const {
		Worker& worker = *workers_[workerIndex];

		while (isRunning_.load() || (isShutdown_.load() && !worker.queue.empty())) {
			std::optional<TaskEntry> task;

			{ // Locked section
				std::unique_lock<std::mutex> lock(worker.mutex);
				worker.cv.wait(lock, [&] {
					return !worker.queue.empty() || !isRunning_.load();
					});

				// Exit if shutdown and no tasks
				if (!isRunning_.load() && worker.queue.empty()) break;

				task = worker.queue.tryPop();
				if (!task) continue;

				// Skip if task was cancelled
				if (task->handle->getIsCancelled()) {
					*worker.taskStates[task->handle->getId()].lock() = TaskState::Cancelled;
					continue;
				}

				*worker.taskStates[task->handle->getId()].lock() = TaskState::Running;
				worker.handle.activeTasks.fetch_add(1);
			}

			// Execute task
			try {
				if (!task->handle->getIsCancelled()) {
					std::visit([&]<typename T>(T & t) {
						if constexpr (std::is_invocable_r_v<void, decltype(t), std::any>) {
							t(std::move(task.value().params));
						}
						else if constexpr (std::is_invocable_r_v<void, decltype(t)>) {
							t();
						}
						else if constexpr (std::is_invocable_r_v<std::any, decltype(t)>) {
							std::dynamic_pointer_cast<TaskHandle>(task->handle)->result = t();
						}
						else if constexpr (std::is_invocable_r_v<std::any, decltype(t), std::any>) {
							std::dynamic_pointer_cast<TaskHandle>(task->handle)->result = t(std::move(task.value().params));
						}
					}, task->task);
				}
			}
			catch (...) {
				std::lock_guard<std::mutex> lock(worker.mutex);
				//TODO....
				*worker.taskStates[task->handle->getId()].lock() = TaskState::Failed;
			}

			{ // Update task state
				std::lock_guard<std::mutex> lock(worker.mutex);
				worker.handle.activeTasks.fetch_sub(1);
				worker.handle.completedTasks.fetch_add(1);
				*worker.taskStates[task->handle->getId()].lock() =
					task->handle->getIsCancelled() ? TaskState::Cancelled : TaskState::Completed;
			}
		}

		// Final cleanup if needed
		if (!isRunning_.load()) {
			std::lock_guard<std::mutex> lock(worker.mutex);
			while (auto task = worker.queue.tryPop()) {
				*worker.taskStates[task->handle->getId()].lock() = TaskState::Cancelled;
			}
		}
	}

	bool DefaultThreadPool::awaitTermination(Nanoseconds timeout) {
		if (isTerminated()) return true; // Already dead

		std::unique_lock<std::mutex> lock(shutdownMutex_);
		return shutdownCV_.wait_for(lock, timeout, [this] { return isTerminated(); });
	}

	void DefaultThreadPool::shutdown() {
		if (isShutdown()) return;
		// Graceful shutdown - complete queued tasks
		isRunning_.store(false);
		isShutdown_.store(true);

		// Wake all workers to process remaining tasks
		for (auto& worker : workers_) {
			std::lock_guard<std::mutex> lock(worker->mutex);
			worker->cv.notify_all();
		}
		for (auto& [id, thread] : threads_) {
			std::visit([&](auto& t) {
				if (t.joinable()) t.join();
				}, thread);
		}
		isTerminated_.store(true);
	}

	void DefaultThreadPool::shutdownNow() {
		if (isShutdown())return;
		// Graceful shutdown - complete queued tasks
		isRunning_.store(false);
		isShutdown_.store(true);

		// Immediate shutdown - cancel all tasks
		for (auto& worker : workers_) {
			std::lock_guard<std::mutex> lock(worker->mutex);

			// Cancel all queued tasks
			while (auto task = worker->queue.tryPop()) {
				task->handle->setCancelled(true);
				*worker->taskStates[task->handle->getId()].lock() = TaskState::Cancelled;
			}

			worker->cv.notify_all();  // Wake all workers
		}

		for (auto& [id, th] : threads_) {
			std::visit([&](auto& t) {
				t.detach();
				}, th);
		}

		// Mark pool as terminated immediately
		isTerminated_.store(true);
		shutdownCV_.notify_all();
	}

	// =============================================
	// Scheduled Thread Pool
	// =============================================
}