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
	using namespace Corium::Memory::Literals;
	inline constexpr Bytes g_TotalVA = CORIUM_VA_ALLOCATION * 1_MiB;

	constexpr Bytes NullGuardSize = Bytes{ 2_MiB };
	constexpr Bytes SectionGuardSize = Bytes{ 2_MiB };

	constexpr size_t MaxTasks = 500'000;
	constexpr uint32_t ParamsPerTask = 8;

	constexpr Bytes RuntimeVASize = Bytes{ 6_GiB };
	constexpr Bytes TaskMetadataVASize = Bytes{ 8_GiB };
	constexpr Bytes TaskPayloadVASize = Bytes{ 14_GiB };


	// -------------------------------------------------------------------------
	// Global VA state
	// -------------------------------------------------------------------------

	inline VirtualSegment g_GlobalMemoryHeaderMemory;
	inline VARegionSlicer g_GlobalVA;

	// -------------------------------------------------------------------------
	// Top-level VA regions
	// -------------------------------------------------------------------------

	inline VARegion g_UpperNullGuard;
	inline VARegion g_RuntimeVA;
	inline VARegion g_RuntimeGuard;
	inline VARegion g_TaskMetadataVA;
	inline VARegion g_TaskMetadataGuard;
	inline VARegion g_TaskPayloadVA;
	inline VARegion g_TaskPayloadGuard;
	inline VARegion g_ReservedVA;
	inline VARegion g_LowerNullGuard;

	// -------------------------------------------------------------------------
	// Runtime / Infra VA sub-regions
	// -------------------------------------------------------------------------

	inline VARegion g_ClosureRange;
	inline VARegion g_ClosureGuard;
	inline VARegion g_SmartPtrControlBlocks;
	inline VARegion g_SmartPtrGuard;
	inline VARegion g_RuntimeCoreObjects;

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Level 1 sections
	// -------------------------------------------------------------------------

	inline VARegion g_TaskObjectLocations;
	inline VARegion g_ObjectLocationsGuard;

	inline VARegion g_TaskInputLayouts;
	inline VARegion g_InputLayoutsGuard;

	inline VARegion g_TaskOutputLayouts;

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Object Locations (typed ranges)
	// -------------------------------------------------------------------------

	inline VARegion g_TaskMemoryDescRange;
	inline VARegion g_TaskMemoryDescGuard;

	inline VARegion g_TaskMemoryHeaderRange;
	inline VARegion g_TaskMemoryHeaderGuard;

	inline VARegion g_TaskContextRange;
	inline VARegion g_TaskContextGuard;

	inline VARegion g_TaskSliceContextRange;
	inline VARegion g_TaskSliceContextGuard;

	inline VARegion g_GPUContextRange;
	inline VARegion g_GPUContextGuard;

	inline VARegion g_ObjectLocationSpare;

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Input Layouts (typed ranges)
	// -------------------------------------------------------------------------

	inline VARegion g_InputSizeArrays;
	inline VARegion g_InputSizeGuard;

	inline VARegion g_InputAlignmentArrays;
	inline VARegion g_InputAlignmentGuard;

	inline VARegion g_InputLayoutSpare;

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Output Layouts (typed ranges)
	// -------------------------------------------------------------------------

	inline VARegion g_OutputSizeArrays;
	inline VARegion g_OutputSizeGuard;

	inline VARegion g_OutputAlignmentArrays;
	inline VARegion g_OutputAlignmentGuard;

	inline VARegion g_OutputLayoutSpare;

	// -------------------------------------------------------------------------
	// Task Payload VA
	// -------------------------------------------------------------------------

	inline VARegion g_TaskPayloadArena;

	// -------------------------------------------------------------------------
	// Initialization
	// -------------------------------------------------------------------------

	struct alignas(64) TaskMemoryHeader final {};
	struct alignas(64) TaskMemoryDescHeader final {};
	struct alignas(64) TaskContextHeader final {};
	struct alignas(64) TaskSliceContextHeader final {};
	struct alignas(64) GPUContextHeader final {};

	ForceInline bool init() {

		g_GlobalMemoryHeaderMemory = VirtualMemory::virtualAlloc(g_GlobalMemoryHeaderMemory,g_TotalVA,
				MemoryOperation::Reserve);

		g_GlobalVA = VARegionSlicer{ g_GlobalMemoryHeaderMemory };

		// --- Top-level VA ----------------------------------------------------

		g_UpperNullGuard = g_GlobalVA.slice(NullGuardSize);

		g_RuntimeVA = g_GlobalVA.slice(RuntimeVASize);
		g_RuntimeGuard = g_GlobalVA.slice(SectionGuardSize);

		g_TaskMetadataVA = g_GlobalVA.slice(TaskMetadataVASize);
		g_TaskMetadataGuard = g_GlobalVA.slice(SectionGuardSize);

		g_TaskPayloadVA = g_GlobalVA.slice(TaskPayloadVASize);
		g_TaskPayloadGuard = g_GlobalVA.slice(SectionGuardSize);

		g_ReservedVA = g_GlobalVA.slice(g_GlobalVA.remaining());
		g_LowerNullGuard = g_GlobalVA.slice(NullGuardSize);

		// --- Runtime VA ------------------------------------------------------

		{
			VARegionSlicer slicer{ g_RuntimeVA };

			g_ClosureRange = slicer.slice(Bytes{ MaxTasks * 3 * 256 });
			g_ClosureGuard = slicer.slice(SectionGuardSize);

			g_SmartPtrControlBlocks = slicer.slice(Bytes{ 2_GiB });
			g_SmartPtrGuard = slicer.slice(SectionGuardSize);

			g_RuntimeCoreObjects = slicer.slice(slicer.remaining());
		}

		// --- TaskMetadata VA -------------------------------------------------

		{
			VARegionSlicer meta{ g_TaskMetadataVA };

			g_TaskObjectLocations = meta.slice(Bytes{ 3_GiB });
			g_ObjectLocationsGuard = meta.slice(SectionGuardSize);

			g_TaskInputLayouts = meta.slice(Bytes{ 3_GiB });
			g_InputLayoutsGuard = meta.slice(SectionGuardSize);

			g_TaskOutputLayouts = meta.slice(meta.remaining());
		}

		// --- Object Locations ------------------------------------------------

		{
			VARegionSlicer obj{ g_TaskObjectLocations };

			g_TaskMemoryDescRange =obj.slice(Bytes{ MaxTasks * sizeof(TaskMemoryDescHeader) });
			g_TaskMemoryDescGuard = obj.slice(SectionGuardSize);

			g_TaskMemoryHeaderRange =obj.slice(Bytes{ MaxTasks * sizeof(TaskMemoryHeader) });
			g_TaskMemoryHeaderGuard = obj.slice(SectionGuardSize);

			g_TaskContextRange =obj.slice(Bytes{ MaxTasks * sizeof(TaskContextHeader) });
			g_TaskContextGuard = obj.slice(SectionGuardSize);

			g_TaskSliceContextRange =obj.slice(Bytes{ MaxTasks * sizeof(TaskSliceContextHeader) });
			g_TaskSliceContextGuard = obj.slice(SectionGuardSize);

			g_GPUContextRange =obj.slice(Bytes{ MaxTasks * sizeof(GPUContextHeader) });
			g_GPUContextGuard = obj.slice(SectionGuardSize);

			g_ObjectLocationSpare =
				obj.slice(obj.remaining());
		}

		// --- Input Layouts ---------------------------------------------------

		{
			VARegionSlicer in{ g_TaskInputLayouts };

			g_InputSizeArrays =in.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
			g_InputSizeGuard = in.slice(SectionGuardSize);

			g_InputAlignmentArrays =in.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
			g_InputAlignmentGuard = in.slice(SectionGuardSize);

			g_InputLayoutSpare =in.slice(in.remaining());
		}

		// --- Output Layouts --------------------------------------------------

		{
			VARegionSlicer out{ g_TaskOutputLayouts };

			g_OutputSizeArrays =out.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
			g_OutputSizeGuard = out.slice(SectionGuardSize);

			g_OutputAlignmentArrays = out.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
			g_OutputAlignmentGuard = out.slice(SectionGuardSize);
			g_OutputLayoutSpare = out.slice(out.remaining());
		}

		// --- Payload VA ------------------------------------------------------

		{
			VARegionSlicer payload{ g_TaskPayloadVA };
			g_TaskPayloadArena = payload.slice(payload.remaining());
		}

		// -----------------------------------------------------------------------------
		// Lock all guard regions (NO ACCESS)
		// -----------------------------------------------------------------------------

		// Top-level guards
		lockGuard(g_UpperNullGuard);
		lockGuard(g_RuntimeGuard);
		lockGuard(g_TaskMetadataGuard);
		lockGuard(g_TaskPayloadGuard);
		lockGuard(g_LowerNullGuard);

		// Runtime / Infra VA guards
		lockGuard(g_ClosureGuard);
		lockGuard(g_SmartPtrGuard);

		// TaskMetadata VA - level 1 guards
		lockGuard(g_ObjectLocationsGuard);
		lockGuard(g_InputLayoutsGuard);

		// TaskMetadata VA - Object Locations guards
		lockGuard(g_TaskMemoryDescGuard);
		lockGuard(g_TaskMemoryHeaderGuard);
		lockGuard(g_TaskContextGuard);
		lockGuard(g_TaskSliceContextGuard);
		lockGuard(g_GPUContextGuard);

		// TaskMetadata VA - Input Layout guards
		lockGuard(g_InputSizeGuard);
		lockGuard(g_InputAlignmentGuard);

		// TaskMetadata VA - Output Layout guards
		lockGuard(g_OutputSizeGuard);
		lockGuard(g_OutputAlignmentGuard);

		return true;
	}
}
