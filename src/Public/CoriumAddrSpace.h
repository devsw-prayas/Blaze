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
|             CORIUM VIRTUAL ADDRESS SPACE  —  512 GiB total
|             4 NUMA nodes × 128 GiB per node, identical layout per node
|             (BY-DEFAULT ALL THE THREAD TLS IS MANAGED BY CORIUM)
==============================================================================

Each node owns a fully independent 128 GiB VA reservation made via
VirtualAllocExNuma, so physical pages committed later stay on that node.
The layout below is replicated identically for every node.

Per-node VA layout (128 GiB):

VA grows upward
^                                                                          ^
|                                                                          |
+--------------------------------------------------------------------------+
|                         UPPER GUARD REGION                               |
|                            (2 MiB, unmapped)                             |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                        RESERVED / FUTURE VA                              |
|                              (~12 GiB)                                   |
|  - GPU staging / DMA windows                                             |
|  - NUMA-local staging buffers                                            |
|  - Sanitizer / shadow memory                                             |
|  - RDMA / remote memory                                                  |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                          LARGE PAGE GUARD                                |
|                              (2 MiB)                                     |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|						CORIUM THREAD LOCAL STORAGE (TLS)				   |
|							  (4 GiB)									   |	
|																		   |
|							TODO!										   |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                          LARGE PAGE GUARD                                |
|                              (2 MiB)                                     |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                          TASK PAYLOAD VA                                 |
|                             (~56 GiB)                                    |
|  - TaskMemory bulk storage                                               |
|  - Input payload buffers                                                 |
|  - Output payload buffers                                                |
|  - Internal / reduction buffers                                          |
|  - GPU-visible task data                                                 |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                          LARGE PAGE GUARD                                |
|                              (2 MiB)                                     |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                         TASK METADATA VA                                 |
|                             (~32 GiB)                                    |
|  - TaskMemoryDesc                                                        |
|  - TaskContext                                                           |
|  - TaskSliceContext                                                      |
|  - GPUContext                                                            |
|  - Alignment buffers (input/output)                                      |
|  - Size buffers (input/output)                                           |
|  - Input / Output buffer views                                           |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                          LARGE PAGE GUARD                                |
|                              (2 MiB)                                     |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                         RUNTIME / INFRA VA                               |
|                             (~24 GiB)                                    |
|  - ClosureFunction objects                                               |
|      * 3 per task (startup / body / shutdown)                            |
|  - Smart pointer control blocks                                          |
|  - Pools                                                                 |
|  - Schedulers                                                            |
|  - Executors                                                             |
|  - Global allocators                                                     |
+--------------------------------------------------------------------------+

+--------------------------------------------------------------------------+
|                          NULL / LOWER GUARD                              |
|                            (2 MiB, unmapped)                             |
|  - Null dereference trap                                                 |
|  - Underflow / bounds violation detection                                |
+--------------------------------------------------------------------------+
|
v
VA grows downward

------------------------------------------------------------------------------
NOTES:
  - Each NUMA node holds an independent reservation — no shared VA base.
  - VirtualAllocExNuma is used per node; physical affinity is set at reserve.
  - TLS memory is split into 2 parts, OS owned TLS that is reserved for call 
  - stacks and Corium TLS that Corium will allocate manually for each thread that
  - is launched
  - Tasks may request TLS size, but TLS allocators are thread-owned.
  - Number of TLS allocators equals number of active worker threads.
  - All regions are contiguous and cache-line aligned internally.
  - 2 MiB guards enforce large-page boundaries and catch linear overruns.
  - Task VA scales with task count per node; TLS scales with thread count.
  - Worker threads are bound to NUMA nodes; allocations always stay node-local.
  - All region arrays are indexed [0 .. MAX_NUMA_NODES - 1] by node id.
------------------------------------------------------------------------------
==============================================================================
*/

namespace Corium::Memory::Internal {
	using namespace Corium::Memory::Literals;

	// -------------------------------------------------------------------------
	// NUMA topology constants
	// -------------------------------------------------------------------------

	constexpr uint32_t MAX_NUMA_NODES = CORIUM_MAX_NUMA;          // maximum supported NUMA nodes
	constexpr Bytes    g_TotalVA = CORIUM_VA_ALLOCATION * 1_MiB;  // 512 GiB total VA budget
	constexpr Bytes    NodeVASize = g_TotalVA / MAX_NUMA_NODES;     // 128 GiB per node

	// -------------------------------------------------------------------------
	// Guard and section sizes
	// -------------------------------------------------------------------------

	constexpr Bytes NullGuardSize = Bytes{ 2_MiB };
	constexpr Bytes SectionGuardSize = Bytes{ 2_MiB };

	// -------------------------------------------------------------------------
	// Per-node task capacity and layout constants
	// -------------------------------------------------------------------------

	constexpr size_t   MaxTasks = 2'000'000;  // maximum concurrent tasks per node
	constexpr uint32_t ParamsPerTask = 8;

	// Per-node region sizes — proportionally scaled 4× from the original 32 GiB layout
	constexpr Bytes ThreadLocalStorageSize = Bytes{ 4_GiB };
	constexpr Bytes RuntimeVASize = Bytes{ 24_GiB };
	constexpr Bytes TaskMetadataVASize = Bytes{ 32_GiB };
	constexpr Bytes TaskPayloadVASize = Bytes{ 56_GiB };

	// Reserved VA per node — computed as whatever is left after all named regions and guards
	constexpr Bytes NodeReservedVASize = NodeVASize
		- NullGuardSize          // upper guard
		- RuntimeVASize
		- SectionGuardSize       // runtime guard
		- TaskMetadataVASize
		- SectionGuardSize       // task metadata guard
		- TaskPayloadVASize
		- SectionGuardSize       // task payload guard
		- NullGuardSize		     // lower guard
		// Addition for FRAME API
		- ThreadLocalStorageSize
		- SectionGuardSize;

	// -------------------------------------------------------------------------
	// Per-node base reservations — one independent VA block per NUMA node
	// -------------------------------------------------------------------------

	extern VirtualSegment g_NodeMemory[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// Top-level VA regions — indexed by NUMA node  (DO NOT TOUCH!)
	// -------------------------------------------------------------------------

	extern VARegion g_UpperNullGuard[MAX_NUMA_NODES];
	extern VARegion g_RuntimeVA[MAX_NUMA_NODES];
	extern VARegion g_RuntimeGuard[MAX_NUMA_NODES];
	extern VARegion g_TaskMetadataVA[MAX_NUMA_NODES];
	extern VARegion g_TaskMetadataGuard[MAX_NUMA_NODES];
	extern VARegion g_TaskPayloadVA[MAX_NUMA_NODES];
	extern VARegion g_TaskPayloadGuard[MAX_NUMA_NODES];
	extern VARegion g_ThreadLocalStorage[MAX_NUMA_NODES];
	extern VARegion g_TLSGuard[MAX_NUMA_NODES];
	extern VARegion g_ReservedVA[MAX_NUMA_NODES];
	extern VARegion g_LowerNullGuard[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// Runtime / Infra VA sub-regions — indexed by NUMA node
	// -------------------------------------------------------------------------

	extern VARegion g_ClosureRange[MAX_NUMA_NODES];
	extern VARegion g_ClosureGuard[MAX_NUMA_NODES];
	extern VARegion g_SmartPtrControlBlocks[MAX_NUMA_NODES];
	extern VARegion g_SmartPtrGuard[MAX_NUMA_NODES];
	extern VARegion g_RuntimeCoreObjects[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Level 1 sections — indexed by NUMA node
	// -------------------------------------------------------------------------

	extern VARegion g_TaskObjectLocations[MAX_NUMA_NODES];
	extern VARegion g_ObjectLocationsGuard[MAX_NUMA_NODES];

	extern VARegion g_TaskInputLayouts[MAX_NUMA_NODES];
	extern VARegion g_InputLayoutsGuard[MAX_NUMA_NODES];

	extern VARegion g_TaskOutputLayouts[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Object Locations (typed ranges) — indexed by NUMA node
	// -------------------------------------------------------------------------

	extern VARegion g_TaskMemoryDescRange[MAX_NUMA_NODES];
	extern VARegion g_TaskMemoryDescGuard[MAX_NUMA_NODES];

	extern VARegion g_TaskMemoryHeaderRange[MAX_NUMA_NODES];
	extern VARegion g_TaskMemoryHeaderGuard[MAX_NUMA_NODES];

	extern VARegion g_TaskContextRange[MAX_NUMA_NODES];
	extern VARegion g_TaskContextGuard[MAX_NUMA_NODES];

	extern VARegion g_TaskSliceContextRange[MAX_NUMA_NODES];
	extern VARegion g_TaskSliceContextGuard[MAX_NUMA_NODES];

	extern VARegion g_GPUContextRange[MAX_NUMA_NODES];
	extern VARegion g_GPUContextGuard[MAX_NUMA_NODES];

	extern VARegion g_ObjectLocationSpare[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Input Layouts (typed ranges) — indexed by NUMA node
	// -------------------------------------------------------------------------

	extern VARegion g_InputSizeArrays[MAX_NUMA_NODES];
	extern VARegion g_InputSizeGuard[MAX_NUMA_NODES];

	extern VARegion g_InputAlignmentArrays[MAX_NUMA_NODES];
	extern VARegion g_InputAlignmentGuard[MAX_NUMA_NODES];

	extern VARegion g_InputLayoutSpare[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Output Layouts (typed ranges) — indexed by NUMA node
	// -------------------------------------------------------------------------

	extern VARegion g_OutputSizeArrays[MAX_NUMA_NODES];
	extern VARegion g_OutputSizeGuard[MAX_NUMA_NODES];

	extern VARegion g_OutputAlignmentArrays[MAX_NUMA_NODES];
	extern VARegion g_OutputAlignmentGuard[MAX_NUMA_NODES];

	extern VARegion g_OutputLayoutSpare[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// Task Payload VA — indexed by NUMA node
	// -------------------------------------------------------------------------

	extern VARegion g_TaskPayloadArena[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// Typed-range sizing stubs — cache-line aligned, used only for sizeof()
	// -------------------------------------------------------------------------

	struct alignas(64) TaskMemoryHeader       final {};
	struct alignas(64) TaskMemoryDescHeader   final {};
	struct alignas(64) TaskContextHeader      final {};
	struct alignas(64) TaskSliceContextHeader final {};
	struct alignas(64) GPUContextHeader       final {};

	// Call this before anything else. Every subsystem depends on it.
	CORIUM_RUNTIME_API bool init();
}
