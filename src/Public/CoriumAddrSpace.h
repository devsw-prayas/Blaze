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
#include <CoriumMemory.h>

// I assume Corium is happy with 32GB of RAM
// It's an assumption, so assume it.

/**
==============================================================================
					CORIUM VIRTUAL ADDRESS SPACE (32 GiB)
					(TLS IS PER-THREAD, NOT HERE)
==============================================================================

VA grows upward
^																		   ^
|																		   |
+--------------------------------------------------------------------------+
|                           UPPER GUARD REGION                             |
|                             (2 MiB, unmapped)                            |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                           RESERVED / FUTURE VA                           |
|                              (~4 GiB)                                    |
|  - GPU staging / DMA windows                                             |
|  - NUMA mirroring                                                        |
|  - Sanitizer / shadow memory                                             |
|  - RDMA / remote memory                                                  |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                           LARGE PAGE GUARD                               |
|                              (2 MiB)                                     |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                           TASK PAYLOAD VA                                |
|                          (~12 - 14 GiB)                                  |
|  - TaskMemory bulk storage                                               |
|  - Input payload buffers                                                 |
|  - Output payload buffers                                                |
|  - Internal / reduction buffers                                          |
|  - GPU-visible task data                                                 |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                           LARGE PAGE GUARD                               |
|                              (2 MiB)                                     |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                           TASK METADATA VA                               |
|                            (~6 - 8 GiB)                                  |
|  - TaskMemoryDesc                                                        |
|  - TaskContext                                                           |
|  - TaskSliceContext                                                      |
|  - GPUContext                                                            |
|  - Alignment buffers (input/output)                                      |
|  - Size buffers (input/output)                                           |
|  - Input / Output buffer views                                           |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                           LARGE PAGE GUARD                               |
|                              (2 MiB)                                     |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                           RUNTIME / INFRA VA                             |
|                            (~4 - 6 GiB)                                  |
|  - ClosureFunction objects                                               |
|      * 3 per task (startup / body / shutdown)                            |
|  - Smart pointer control blocks                                          |
|  - Pools                                                                 |
|  - Schedulers                                                            |
|  - Executors                                                             |
|  - Global allocators                                                     |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                           LARGE PAGE GUARD                               |
|                              (2 MiB)                                     |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                           NULL / LOWER GUARD                             |
|                           (256 MiB, unmapped)                            |
|  - Null dereference trap                                                 |
|  - Underflow / bounds violation detection                                |
+--------------------------------------------------------------------------+
|
v
VA grows downward

------------------------------------------------------------------------------
NOTES:
  - TLS memory is owned by worker threads and is NOT part of this VA.
  - Tasks may request TLS size, but TLS allocators are thread-owned.
  - Number of TLS allocators equals number of active worker threads.
  - All regions are contiguous and cache-line aligned internally.
  - 2 MiB guards enforce large-page boundaries and catch linear overruns.
  - Task VA scales with task count; TLS scales with thread count.
------------------------------------------------------------------------------
==============================================================================
*/



namespace Corium::Memory::Internal {

	// 3 million different VA slices, what the fuck?!

	using namespace Corium::Memory::Literals;
	constexpr Bytes g_TotalVA = CORIUM_VA_ALLOCATION * 1_MiB;

	constexpr Bytes NullGuardSize = Bytes{ 2_MiB };
	constexpr Bytes SectionGuardSize = Bytes{ 2_MiB };

	constexpr size_t MaxTasks = 500'000;
	constexpr uint32_t ParamsPerTask = 8;

	constexpr Bytes RuntimeVASize = Bytes{ 6_GiB };
	constexpr Bytes TaskMetadataVASize = Bytes{ 8_GiB };
	constexpr Bytes TaskPayloadVASize = Bytes{ 14_GiB };

	constexpr Bytes ReservedVASize = g_TotalVA
		- NullGuardSize          // upper null guard
		- RuntimeVASize
		- SectionGuardSize       // runtime guard
		- TaskMetadataVASize
		- SectionGuardSize       // task metadata guard
		- TaskPayloadVASize
		- SectionGuardSize       // task payload guard
		- NullGuardSize;         // lower null guard


	// -------------------------------------------------------------------------
	// Global VA state
	// -------------------------------------------------------------------------

	extern VirtualSegment g_GlobalMemoryHeaderMemory;
	extern VARegionSlicer g_GlobalVA;

	// -------------------------------------------------------------------------
	// Top-level VA regions		(DO NOT TOUCH!)
	// -------------------------------------------------------------------------

	extern VARegion g_UpperNullGuard;
	extern VARegion g_RuntimeVA;
	extern VARegion g_RuntimeGuard;
	extern VARegion g_TaskMetadataVA;
	extern VARegion g_TaskMetadataGuard;
	extern VARegion g_TaskPayloadVA;
	extern VARegion g_TaskPayloadGuard;
	extern VARegion g_ReservedVA;
	extern VARegion g_LowerNullGuard;

	// -------------------------------------------------------------------------
	// Runtime / Infra VA sub-regions
	// -------------------------------------------------------------------------

	extern VARegion g_ClosureRange;
	extern VARegion g_ClosureGuard;
	extern VARegion g_SmartPtrControlBlocks;
	extern VARegion g_SmartPtrGuard;
	extern VARegion g_RuntimeCoreObjects;

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Level 1 sections
	// -------------------------------------------------------------------------

	extern VARegion g_TaskObjectLocations;
	extern VARegion g_ObjectLocationsGuard;

	extern VARegion g_TaskInputLayouts;
	extern VARegion g_InputLayoutsGuard;

	extern VARegion g_TaskOutputLayouts;

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Object Locations (typed ranges)
	// -------------------------------------------------------------------------

	extern VARegion g_TaskMemoryDescRange;
	extern VARegion g_TaskMemoryDescGuard;

	extern VARegion g_TaskMemoryHeaderRange;
	extern VARegion g_TaskMemoryHeaderGuard;

	extern VARegion g_TaskContextRange;
	extern VARegion g_TaskContextGuard;

	extern VARegion g_TaskSliceContextRange;
	extern VARegion g_TaskSliceContextGuard;

	extern VARegion g_GPUContextRange;
	extern VARegion g_GPUContextGuard;

	extern VARegion g_ObjectLocationSpare;

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Input Layouts (typed ranges)
	// -------------------------------------------------------------------------

	extern VARegion g_InputSizeArrays;
	extern VARegion g_InputSizeGuard;

	extern VARegion g_InputAlignmentArrays;
	extern VARegion g_InputAlignmentGuard;

	extern VARegion g_InputLayoutSpare;

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Output Layouts (typed ranges)
	// -------------------------------------------------------------------------

	extern VARegion g_OutputSizeArrays;
	extern VARegion g_OutputSizeGuard;

	extern VARegion g_OutputAlignmentArrays;
	extern VARegion g_OutputAlignmentGuard;

	extern VARegion g_OutputLayoutSpare;

	// -------------------------------------------------------------------------
	// Task Payload VA
	// -------------------------------------------------------------------------

	extern VARegion g_TaskPayloadArena;

	// -------------------------------------------------------------------------
	// Initialization
	// -------------------------------------------------------------------------

	// Prevents the 67th cyclic dependecy

	struct alignas(64) TaskMemoryHeader final {};
	struct alignas(64) TaskMemoryDescHeader final {};
	struct alignas(64) TaskContextHeader final {};
	struct alignas(64) TaskSliceContextHeader final {};
	struct alignas(64) GPUContextHeader final {};

	// Call this, or ur life be fucked hard
	CORIUM_RUNTIME_API bool init();
}
