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

namespace Corium::Backend::Traits {
	using Flag = bool;
	Flag constexpr Allow = true;
	Flag constexpr Disallow = false;

#ifndef DefineTrait
#define DefineTrait(name) \
	template<typename T, typename = void> \
	struct CORIUM name final : std::false_type {};\
	\
	template<typename T> \
	ForceInline bool CORIUM constexpr name##V = name<T>::value; \

#endif

#ifndef DefineConditional
#define DefineConditional(name) \
	template<Flag permission> \
	struct CORIUM name final : std::conditional<permission, std::true_type, std::false_type> {};

#endif

	// ─────────────────────────────────────────────
	// Execution substrate & backend capabilities
	// ─────────────────────────────────────────────

	// SupportsNativeExecution
	// Asserts: Backend is capable of executing work on OS-level hardware threads.
	// Does NOT imply: Virtualized execution is unavailable or disallowed.
	DefineTrait(SupportsNativeExecution)

		// SupportsVirtualizedExecution
		// Asserts: Backend can execute work on virtualized execution units (fibers/coroutines).
		// Does NOT imply: Native execution is unavailable or that tasks are virtualization-safe.
		DefineTrait(SupportsVirtualizedExecution)

		// SupportsAcceleratorExecution
		// Asserts: Backend can execute tasks on non-CPU hardware accelerators.
		// Does NOT imply: All tasks support acceleration or that CPU fallback is allowed.
		DefineTrait(SupportsAcceleratorExecution)

		// ─────────────────────────────────────────────
		// Execution model & scheduling capabilities
		// ─────────────────────────────────────────────

		// SupportsImmediateExecution
		// Asserts: Tasks may become eligible for execution immediately upon submission.
		// Does NOT imply: Synchronous execution or blocking behavior.
		DefineConditional(SupportsImmediateExecution)

		// SupportsDelayedExecution
		// Asserts: Backend supports time-based delayed execution eligibility.
		// Does NOT imply: Real-time guarantees or periodic scheduling.
		DefineConditional(SupportsDelayedExecution)

		// SupportsPeriodicExecution
		// Asserts: Backend supports recurring, time-based execution eligibility.
		// Does NOT imply: Deterministic timing or work-stealing behavior.
		DefineConditional(SupportsPeriodicExecution)

		// SupportsWorkStealingExecution
		// Asserts: Backend supports work stealing across execution units.
		// Does NOT imply: All tasks are eligible for stealing or automatic decomposition.
		DefineConditional(SupportsWorkStealingExecution)

		// ─────────────────────────────────────────────
		// Execution width & affinity guarantees
		// ─────────────────────────────────────────────

		// SupportsSmtWidening
		// Asserts: Backend may widen a single execution across SMT hardware lanes.
		// Does NOT imply: Task decomposition, work stealing, or multiple logical tasks.
		DefineConditional(SupportsSmtWidening)

		// SupportsSingleLaneExecution
		// Asserts: Backend may restrict execution to a single hardware lane.
		// Does NOT imply: Global serialization or deterministic scheduling.
		DefineConditional(SupportsSingleLaneExecution)

		// GuaranteesThreadAffinity
		// Asserts: Execution remains bound to a single worker thread for its lifetime.
		// Does NOT imply: Locking, exclusivity, or blocking semantics.
		DefineConditional(GuaranteesThreadAffinity)

		// GuaranteesStateIsolation
		// Asserts: Execution-local state does not leak across task boundaries.
		// Does NOT imply: Determinism or absence of shared memory.
		DefineConditional(GuaranteesStateIsolation)

		// ─────────────────────────────────────────────
		// TLS & lifetime guarantees
		// ─────────────────────────────────────────────

		// EnsuresTlsIsolation
		// Asserts: Execution uses thread-/fiber-local storage isolated per execution.
		// Does NOT imply: TLS is reset between executions or that TLS is zero-initialized.
		DefineConditional(EnsuresTlsIsolation)

		// EnsuresTlsWipeReset
		// Asserts: TLS state is reset or wiped after each execution completes.
		// Does NOT imply: TLS is unavailable during execution.
		DefineConditional(EnsuresTlsWipeReset)

		// ProvidesTlsLocalArena
		// Asserts: A per-execution scratch allocator is available via TLS.
		// Does NOT imply: Unbounded allocation or persistence across executions.
		DefineConditional(ProvidesTlsLocalArena)

		// ─────────────────────────────────────────────
		// Hard execution prohibitions
		// ─────────────────────────────────────────────

		// ForbidsBlockingExecution
		// Asserts: Executions must not perform blocking operations.
		// Does NOT imply: Tasks are non-blocking by nature or that blocking is detected.
		DefineConditional(ForbidsBlockingExecution)

		// ForbidsTaskMigration
		// Asserts: Execution must not migrate across workers once started.
		// Does NOT imply: Thread affinity unless explicitly guaranteed elsewhere.
		DefineConditional(ForbidsTaskMigration)

		// ForbidsSoftwareFallback
		// Asserts: Execution must not fall back to CPU if accelerator execution is unavailable.
		// Does NOT imply: Accelerator execution is always available.
		DefineConditional(ForbidsSoftwareFallback)

		// ForbidsPathwayRescheduling
		// Asserts: Execution pathway is fixed at submission time.
		// Does NOT imply: Deterministic scheduling or execution order.
		DefineConditional(ForbidsPathwayRescheduling)

		// ─────────────────────────────────────────────
		// Task structural & behavioral metadata
		// ─────────────────────────────────────────────
		// SingleShotTask
		// Asserts: Task is intended to execute exactly once and then retire.
		// Does NOT imply: Lack of internal looping or sub-executions.
		DefineConditional(SingleShotTask)

		// RepeatableTask
		// Asserts: Task is intended to execute multiple times across its lifetime.
		// Does NOT imply: Periodic or time-based scheduling.
		DefineConditional(RepeatableTask)

		// WorkStealableTask
		// Asserts: Task may be decomposed and migrated via work stealing.
		// Does NOT imply: Internal parallelism or streaming behavior.
		DefineConditional(WorkStealableTask)

		// StreamingTask
		// Asserts: Task operates over large, sequential or streaming data.
		// Does NOT imply: Work stealing or task decomposition.
		DefineConditional(StreamingTask)

		// ComposableTask
		// Asserts: Task may participate in async composition chains.
		// Does NOT imply: Automatic scheduling or continuation guarantees.
		DefineConditional(ComposableTask)

		// ─────────────────────────────────────────────
		// Task temporal semantics
		// ─────────────────────────────────────────────

		// ImmediateTask
		// Asserts: Task is eligible for execution immediately upon submission.
		// Does NOT imply: Synchronous or blocking execution.
		DefineConditional(ImmediateTask)

		// FutureTask
		// Asserts: Task becomes eligible only after a future condition is satisfied.
		// Does NOT imply: Time-based or periodic execution.
		DefineConditional(FutureTask)

		// PeriodicTask
		// Asserts: Task is intended to execute repeatedly at fixed temporal intervals.
		// Does NOT imply: Precise timing guarantees.
		DefineConditional(PeriodicTask)

		// ImplicitDataFlush
		// Asserts: Task-owned metadata and memory are reclaimed automatically by the pool upon reaching a terminal execution state.
		// Does NOT imply: Output ownership transfer, post-completion observability, or that task effects are reversible or inspectable.
		DefineConditional(ImplicitDataFlush)

		// ExplicitDataFlush
		// Asserts: Task-owned metadata and memory persist after execution completes until explicitly released by the user via finalization.
		// Does NOT imply: Automatic lifetime extension, safety after finalization, or that resource reclamation is enforced or guaranteed.
		DefineConditional(ExplicitDataFlush)

		// ─────────────────────────────────────────────
		// Task concurrency & migration semantics
		// ─────────────────────────────────────────────

		// ExclusiveTask
		// Asserts: Task requires exclusive access to its execution unit.
		// Does NOT imply: Global exclusivity or mutual exclusion across all tasks.
		DefineConditional(ExclusiveTask)

		// NonStealableTask
		// Asserts: Task must not be migrated or stolen across workers.
		// Does NOT imply: Thread affinity unless separately guaranteed.
		DefineConditional(NonStealableTask)

		// MigratableTask
		// Asserts: Task execution may migrate across workers.
		// Does NOT imply: Work stealing or decomposition eligibility.
		DefineConditional(MigratableTask)

		// ─────────────────────────────────────────────
		// Task execution domain & safety descriptors
		// ─────────────────────────────────────────────

		// SoftwareTask
		// Asserts: Task must execute on CPU and has no accelerator implementation.
		// Does NOT imply: Blocking or non-blocking behavior.
		DefineConditional(SoftwareTask)

		// AcceleratorTask
		// Asserts: Task has an accelerator-capable implementation.
		// Does NOT imply: CPU fallback is forbidden.
		DefineConditional(AcceleratorTask)

		// VirtualizationSafeTask
		// Asserts: Task behavior is correct under virtualized execution.
		// Does NOT imply: Virtualized execution is required or preferred.
		DefineConditional(VirtualizationSafeTask)

		// BlockingTask
		// Asserts: Task may perform blocking operations.
		// Does NOT imply: Scheduler rejection or serialization.
		DefineConditional(BlockingTask)

		template<typename T, bool Permissions, typename = void>
	struct IsMutatorConditional final : std::false_type {};

	template<bool P>
	struct IsMutatorConditional<SupportsImmediateExecution<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<SupportsDelayedExecution<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<SupportsPeriodicExecution<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<SupportsWorkStealingExecution<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<SupportsSmtWidening<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<SupportsSingleLaneExecution<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<GuaranteesThreadAffinity<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<GuaranteesStateIsolation<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<EnsuresTlsIsolation<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<EnsuresTlsWipeReset<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<ProvidesTlsLocalArena<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<ForbidsBlockingExecution<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<ForbidsTaskMigration<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<ForbidsSoftwareFallback<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<ForbidsPathwayRescheduling<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<SingleShotTask<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<RepeatableTask<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<WorkStealableTask<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<StreamingTask<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<ComposableTask<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<ImmediateTask<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<FutureTask<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<PeriodicTask<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<ImplicitDataFlush<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<ExplicitDataFlush<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<ExclusiveTask<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<NonStealableTask<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<MigratableTask<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<SoftwareTask<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<AcceleratorTask<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<VirtualizationSafeTask<P>, P> : std::true_type {};

	template<bool P>
	struct IsMutatorConditional<BlockingTask<P>, P> : std::true_type {};


}