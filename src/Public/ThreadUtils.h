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
#include <CoriumUtility.h>

namespace Corium::Core {
	struct Allocator {};

	using AffinityMask = size_t;
	using ProcessorIdx = uint32_t;
	using ThreadPriority = int32_t;

	enum class DescriptorState : uint8_t { UNINITIALIZED, MUTABLE, FROZEN };

	struct CORIUM_RUNTIME_API alignas(64) ThreadAttrDesc final {
		AffinityMask m_Mask;
		ProcessorIdx m_IdealProcessor;
		ThreadPriority m_ThreadPriority;
		DescriptorState m_State = DescriptorState::UNINITIALIZED;
		bool m_isDetached;
		bool m_SupportsIdealProcessor;
		bool m_IsGuardPageEnabled;

		ThreadAttrDesc() = default;
		~ThreadAttrDesc() = default;

		ThreadAttrDesc(const ThreadAttrDesc&) = default;
		ThreadAttrDesc& operator=(const ThreadAttrDesc&) = default;
		ThreadAttrDesc(ThreadAttrDesc&&) noexcept = default;
		ThreadAttrDesc& operator=(ThreadAttrDesc&&) noexcept = default;
	};

	void CORIUM_RUNTIME_API init(ThreadAttrDesc& ro_Desc);
	void CORIUM_RUNTIME_API setAffinity(ThreadAttrDesc& ro_Desc, AffinityMask v_Mask);
	void CORIUM_RUNTIME_API shouldSupportIdealProcessor(ThreadAttrDesc& ro_Desc, bool v_Permission);
	void CORIUM_RUNTIME_API setIdealProcessor(ThreadAttrDesc& ro_Desc, ProcessorIdx v_Idx);
	void CORIUM_RUNTIME_API canDetach(ThreadAttrDesc& ro_Desc, bool v_Permission);
	void CORIUM_RUNTIME_API vaGuardEnabled(ThreadAttrDesc& ro_Desc, bool v_Permission);
	bool CORIUM_RUNTIME_API validate(ThreadAttrDesc& ro_Desc);

	struct CORIUM_RUNTIME_API alignas(64) ThreadLaunchDesc final {
		Utils::ClosureFunction<Allocator, void()> m_StartPoint;
		size_t m_VaSize;
		const char* m_Name;
		DescriptorState m_State;
		bool m_IsPreSuspended;
	};

	void CORIUM_RUNTIME_API init(ThreadLaunchDesc& ro_Desc);
	void CORIUM_RUNTIME_API
		attachLaunchAddr(ThreadLaunchDesc& ro_Desc,
						 Utils::ClosureFunction<Allocator, void()> v_Closure);
	void CORIUM_RUNTIME_API setVaSize(ThreadLaunchDesc& ro_Desc, size_t v_VaSize);
	void CORIUM_RUNTIME_API setName(ThreadLaunchDesc& ro_Desc, const char* p_Name);
	void CORIUM_RUNTIME_API isPreSuspended(ThreadLaunchDesc& ro_Desc, bool v_Permission);
	void CORIUM_RUNTIME_API validate(ThreadLaunchDesc& ro_Desc);

	CORIUM_FORCEINLINE static bool isFrozen(const ThreadAttrDesc& ro_Desc) {
		return ro_Desc.m_State == DescriptorState::FROZEN;
	}

	CORIUM_FORCEINLINE static void promoteMutable(ThreadAttrDesc& r_Desc) {
		if (r_Desc.m_State == DescriptorState::UNINITIALIZED)
			r_Desc.m_State = DescriptorState::MUTABLE;
	}
} 
