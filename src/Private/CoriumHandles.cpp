#include "Corium.h"
#include "CoriumHandles.h"

namespace Corium::Utils {
	bool ActionHandle::cancelTask() noexcept {
		auto state = m_State.load(std::memory_order_relaxed);
		if (state == TaskState::CANCELLED || state == TaskState::COMPLETE || state == TaskState::FAILED) return false;
		m_State.store(TaskState::CANCELLED, std::memory_order_seq_cst);
		return true;
	}

	std::exception_ptr ActionHandle::getException() const noexcept {
		return m_Exception;
	}

	void ActionHandle::rethrow() const {
		rethrow_exception(m_Exception);
	}

	size_t ActionHandle::getTaskID() const noexcept {
		return m_TaskID;
	}

	TaskState ActionHandle::getState() const noexcept {
		return m_State;
	}

	template<typename T>
	bool TaskHandle<T>::cancelTask() noexcept {
		auto state = m_State.load(std::memory_order_relaxed);
		if (state == TaskState::CANCELLED || state == TaskState::COMPLETE || state == TaskState::FAILED) return false;
		m_State.store(TaskState::CANCELLED, std::memory_order_seq_cst);
		return true;
	}

	template<typename T>
	std::exception_ptr TaskHandle<T>::getException() const noexcept {
		return m_Exception;
	}

	template<typename T>
	void TaskHandle<T>::rethrow() const {
		rethrow_exception(m_Exception);
	}

	template<typename T>
	size_t TaskHandle<T>::getTaskID() const noexcept {
		return m_TaskID;
	}

	template<typename T>
	TaskState TaskHandle<T>::getState() const noexcept {
		return m_State;
	}
}
