#pragma once
#include <Corium.h>

#include "CoriumMemory.h"
#include "Inductor.h"

namespace Corium::Execution::Builder {
	struct TaskMemory final {
		TaskMemory() {}
		~TaskMemory() = default;

		TaskMemory(const TaskMemory&) = delete;
		TaskMemory& operator=(const TaskMemory&) = delete;

		TaskMemory(TaskMemory&&) noexcept = default;
		TaskMemory& operator=(TaskMemory&&) noexcept = default;

	private:
		Memory::UniquePtr<void> m_InputMemBuffer;
		Memory::UniquePtr<void> m_OutputMemBuffer;
	};

	struct CORIUM TaskFrame final {
		TaskFrame() {}
		~TaskFrame() = default;
		TaskFrame(const TaskFrame&) = default;
		TaskFrame& operator=(const TaskFrame&) = delete;

		TaskFrame(TaskFrame&&) noexcept = default;
		TaskFrame& operator=(TaskFrame&&) noexcept = delete;

	private:
		const Memory::WeakPtr<Inductor::TaskMemoryDesc> m_MemDesc;
		friend struct TaskMemoryDesc;
	};

	class CORIUM TaskBuilder final {

	};
} // namespace Corium::Execution::Builder
