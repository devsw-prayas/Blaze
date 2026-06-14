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
#include <Corium.h>

#include "CoriumMemory.h"
#include "Inductor.h"

namespace Corium::Execution::Builder {

	struct TaskMemory final {
		TaskMemory() = default;
		~TaskMemory() = default;

		TaskMemory(const TaskMemory&) = delete;
		TaskMemory& operator=(const TaskMemory&) = delete;

		TaskMemory(TaskMemory&&) noexcept = default;
		TaskMemory& operator=(TaskMemory&&) noexcept = default;

	private:
		Memory::UniquePtr<void> m_InputMemBuffer;
		Memory::UniquePtr<void> m_OutputMemBuffer;
	};

	struct CORIUM_RUNTIME_API TaskFrame final {
		TaskFrame() = default;
		~TaskFrame() = default;

		explicit TaskFrame(
			Memory::WeakPtr<Inductor::TaskMemoryDesc, Memory::Allocators::TaskMetadataAllocator> sw_Desc
		) noexcept : m_MemDesc(std::move(sw_Desc)) {}

		TaskFrame(const TaskFrame&) = default;
		TaskFrame& operator=(const TaskFrame&) = delete;

		TaskFrame(TaskFrame&&) noexcept = default;
		TaskFrame& operator=(TaskFrame&&) noexcept = delete;

	private:
		Memory::WeakPtr<Inductor::TaskMemoryDesc, Memory::Allocators::TaskMetadataAllocator> m_MemDesc;

		friend class TaskBuilder;
	};

	class CORIUM_RUNTIME_API TaskBuilder final {
	public:
		static TaskFrame build(const Inductor::TaskDesc& ro_Desc);
	};

} // namespace Corium::Execution::Builder
