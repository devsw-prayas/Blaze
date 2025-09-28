#pragma once
#include "CoriumUtils.h"

namespace Corium::Utils {
	struct CORIUM alignas(64) ActionHandle final : IHandle{
		std::atomic<TaskState> m_State;
		std::atomic<std::exception_ptr> m_Exception;
		std::atomic<size_t> m_TaskID;

		explicit ActionHandle(size_t v_TaskID) noexcept
			:m_State(TaskState::PENDING), m_Exception(nullptr), m_TaskID(v_TaskID) {
}

		bool cancelTask() noexcept override;
		std::exception_ptr getException() const noexcept override;
		TaskState getState() const noexcept override;
		size_t getTaskID() const noexcept override;
		void rethrow() const override;

		template<typename T>
		void resultC() noexcept {
			/*No-Op*/
		}
	};

	template<typename T>
	struct CORIUM alignas(64) TaskHandle final : IHandle{
		std::atomic<TaskState> m_State;
		std::atomic<std::exception_ptr> m_Exception;
		std::atomic<size_t> m_TaskID;
		T m_Result;

		explicit TaskHandle(size_t v_TaskID) noexcept
			:m_State(TaskState::PENDING), m_Exception(nullptr), m_TaskID(v_TaskID), m_Result() {
}

		bool cancelTask() noexcept override;
		std::exception_ptr getException() const noexcept override;
		TaskState getState() const noexcept override;
		size_t getTaskID() const noexcept override;
		void rethrow() const override;

		T* result() {
			if (m_State.load(std::memory_order_relaxed) == TaskState::COMPLETE)
				return &m_Result;
			return nullptr;
		}
	};
}