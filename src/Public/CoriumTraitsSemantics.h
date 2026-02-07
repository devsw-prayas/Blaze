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
#include <CoriumTraits.h>

// Do not touch this enum values, or there will be some very nasty consequences during frontend
// decomposition

namespace Corium::Backend::Traits {
	enum class TaskBits : uint64_t {
		// ─────────────────────────────────────────────
		// Scheduling & eligibility (8 to 15)
		// ─────────────────────────────────────────────
		SupportsImmediateExecution = 1ull << 8,
		SupportsDelayedExecution = 1ull << 9,
		SupportsPeriodicExecution = 1ull << 10,
		SupportsWorkStealingExecution = 1ull << 11,

		// ─────────────────────────────────────────────
		// Execution width & affinity (16 to 23)
		// ─────────────────────────────────────────────

		SupportsSmtWidening = 1ull << 16,
		SupportsSingleLaneExecution = 1ull << 17,
		GuaranteesThreadAffinity = 1ull << 18,
		GuaranteesStateIsolation = 1ull << 19,

		// ─────────────────────────────────────────────
		// TLS & lifetime (24 to 31)
		// ─────────────────────────────────────────────

		EnsuresTlsIsolation = 1ull << 24,
		EnsuresTlsWipeReset = 1ull << 25,
		ProvidesTlsLocalArena = 1ull << 26,

		// ─────────────────────────────────────────────
		// Hard execution prohibitions (32 to 39)
		// ─────────────────────────────────────────────

		ForbidsBlockingExecution = 1ull << 32,
		ForbidsTaskMigration = 1ull << 33,
		ForbidsSoftwareFallback = 1ull << 34,
		ForbidsPathwayRescheduling = 1ull << 35,

		// ─────────────────────────────────────────────
		// Task structure & behavior (40 to 47)
		// ─────────────────────────────────────────────

		SingleShotTask = 1ull << 40,
		RepeatableTask = 1ull << 41,
		WorkStealableTask = 1ull << 42,
		StreamingTask = 1ull << 43,
		ComposableTask = 1ull << 44,

		// ─────────────────────────────────────────────
		// Temporal semantics (48 to 55)
		// ─────────────────────────────────────────────

		ImmediateTask = 1ull << 48,
		FutureTask = 1ull << 49,
		PeriodicTask = 1ull << 50,
		ImplicitDataFlush = 1ull << 51,
		ExplicitDataFlush = 1ull << 52,

		// ─────────────────────────────────────────────
		// Domain & safety (56 to 63)
		// ─────────────────────────────────────────────

		ExclusiveTask = 1ull << 56,
		NonStealableTask = 1ull << 57,
		MigratableTask = 1ull << 58,
		SoftwareTask = 1ull << 59,
		AcceleratorTask = 1ull << 60,
		VirtualizationSafeTask = 1ull << 61,
		BlockingTask = 1ull << 62,

		Invalid = 0ull
	};

	template<typename Trait>
	struct TraitToBit;

#ifndef MapToBit
#define MapToBit(Trait, Permission) \
		template<> \
		struct TraitToBit<Trait<Permission>> {static  constexpr TaskBits  value = Trait<Permission>::type::value ? TaskBits::Trait : TaskBits::Invalid; }; \

#endif
	MapToBit(SupportsImmediateExecution, Allow)
		MapToBit(SupportsImmediateExecution, Disallow)

		MapToBit(SupportsDelayedExecution, Allow)
		MapToBit(SupportsDelayedExecution, Disallow)

		MapToBit(SupportsPeriodicExecution, Allow)
		MapToBit(SupportsPeriodicExecution, Disallow)

		MapToBit(SupportsWorkStealingExecution, Allow)
		MapToBit(SupportsWorkStealingExecution, Disallow)

		MapToBit(SupportsSmtWidening, Allow)
		MapToBit(SupportsSmtWidening, Disallow)

		MapToBit(SupportsSingleLaneExecution, Allow)
		MapToBit(SupportsSingleLaneExecution, Disallow)

		MapToBit(GuaranteesThreadAffinity, Allow)
		MapToBit(GuaranteesThreadAffinity, Disallow)

		MapToBit(GuaranteesStateIsolation, Allow)
		MapToBit(GuaranteesStateIsolation, Disallow)

		MapToBit(EnsuresTlsIsolation, Allow)
		MapToBit(EnsuresTlsIsolation, Disallow)

		MapToBit(EnsuresTlsWipeReset, Allow)
		MapToBit(EnsuresTlsWipeReset, Disallow)

		MapToBit(ProvidesTlsLocalArena, Allow)
		MapToBit(ProvidesTlsLocalArena, Disallow)

		MapToBit(ForbidsBlockingExecution, Allow)
		MapToBit(ForbidsBlockingExecution, Disallow)

		MapToBit(ForbidsTaskMigration, Allow)
		MapToBit(ForbidsTaskMigration, Disallow)

		MapToBit(ForbidsSoftwareFallback, Allow)
		MapToBit(ForbidsSoftwareFallback, Disallow)

		MapToBit(ForbidsPathwayRescheduling, Allow)
		MapToBit(ForbidsPathwayRescheduling, Disallow)

		MapToBit(SingleShotTask, Allow)
		MapToBit(SingleShotTask, Disallow)

		MapToBit(RepeatableTask, Allow)
		MapToBit(RepeatableTask, Disallow)

		MapToBit(WorkStealableTask, Allow)
		MapToBit(WorkStealableTask, Disallow)

		MapToBit(StreamingTask, Allow)
		MapToBit(StreamingTask, Disallow)

		MapToBit(ComposableTask, Allow)
		MapToBit(ComposableTask, Disallow)

		MapToBit(ImmediateTask, Allow)
		MapToBit(ImmediateTask, Disallow)

		MapToBit(FutureTask, Allow)
		MapToBit(FutureTask, Disallow)

		MapToBit(PeriodicTask, Allow)
		MapToBit(PeriodicTask, Disallow)

		template<typename ...Traits>
	struct TraitsSet final {};

	constexpr uint64_t toMask(TaskBits bit) {
		return static_cast<uint64_t>(bit);
	}

	template<typename... Traits>
	consteval uint64_t buildTaskMask(TraitsSet<Traits...>) {
		return (0ull | ... | toMask(TraitToBit<Traits>::value));
	}
}
