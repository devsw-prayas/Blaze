#include "Corium.h"
#include "CoriumAddrSpace.h"

namespace Corium::Memory::Internal {

	// -------------------------------------------------------------------------
	// Global VA state
	// -------------------------------------------------------------------------

	VirtualSegment  g_GlobalMemoryHeaderMemory;
	VARegionSlicer  g_GlobalVA;

	// -------------------------------------------------------------------------
	// Top-level VA regions
	// -------------------------------------------------------------------------

	VARegion g_UpperNullGuard;
	VARegion g_RuntimeVA;
	VARegion g_RuntimeGuard;
	VARegion g_TaskMetadataVA;
	VARegion g_TaskMetadataGuard;
	VARegion g_TaskPayloadVA;
	VARegion g_TaskPayloadGuard;
	VARegion g_ReservedVA;
	VARegion g_LowerNullGuard;

	// -------------------------------------------------------------------------
	// Runtime / Infra VA sub-regions
	// -------------------------------------------------------------------------

	VARegion g_ClosureRange;
	VARegion g_ClosureGuard;
	VARegion g_SmartPtrControlBlocks;
	VARegion g_SmartPtrGuard;
	VARegion g_RuntimeCoreObjects;

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Level 1 sections
	// -------------------------------------------------------------------------

	VARegion g_TaskObjectLocations;
	VARegion g_ObjectLocationsGuard;

	VARegion g_TaskInputLayouts;
	VARegion g_InputLayoutsGuard;

	VARegion g_TaskOutputLayouts;

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Object Locations (typed ranges)
	// -------------------------------------------------------------------------

	VARegion g_TaskMemoryDescRange;
	VARegion g_TaskMemoryDescGuard;

	VARegion g_TaskMemoryHeaderRange;
	VARegion g_TaskMemoryHeaderGuard;

	VARegion g_TaskContextRange;
	VARegion g_TaskContextGuard;

	VARegion g_TaskSliceContextRange;
	VARegion g_TaskSliceContextGuard;

	VARegion g_GPUContextRange;
	VARegion g_GPUContextGuard;

	VARegion g_ObjectLocationSpare;

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Input Layouts (typed ranges)
	// -------------------------------------------------------------------------

	VARegion g_InputSizeArrays;
	VARegion g_InputSizeGuard;

	VARegion g_InputAlignmentArrays;
	VARegion g_InputAlignmentGuard;

	VARegion g_InputLayoutSpare;

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Output Layouts (typed ranges)
	// -------------------------------------------------------------------------

	VARegion g_OutputSizeArrays;
	VARegion g_OutputSizeGuard;

	VARegion g_OutputAlignmentArrays;
	VARegion g_OutputAlignmentGuard;

	VARegion g_OutputLayoutSpare;

	// -------------------------------------------------------------------------
	// Task Payload VA
	// -------------------------------------------------------------------------

	VARegion g_TaskPayloadArena;

	// -------------------------------------------------------------------------
	// init()
	// -------------------------------------------------------------------------

	bool init() {
		// ==============================================================================
		//                     CORIUM VIRTUAL ADDRESS SPACE HIERARCHY
		// ==============================================================================
		//
		// GLOBAL VA (32 GiB reserved)
		// |
		// |  UPPER NULL GUARD (2 MiB)
		// |
		// |- RUNTIME / INFRASTRUCTURE VA (~6 GiB)
		// |  |
		// |  |- ClosureRange
		// |  |    - ClosureFunction objects
		// |  |    - 3 closures per task (startup / body / shutdown)
		// |  |
		// |  |- Guard (2 MiB)
		// |  |
		// |  |- SmartPtrControlBlocks (2 GiB)
		// |  |    - shared_ptr / intrusive control blocks
		// |  |
		// |  |- Guard (2 MiB)
		// |  |
		// |  |- RuntimeCoreObjects
		// |       - schedulers
		// |       - executors
		// |       - pools
		// |       - global allocators
		// |
		// |- RUNTIME GUARD (2 MiB)
		// |
		// |- TASK METADATA VA (~8 GiB)
		// |  |
		// |  |- Object Locations (3 GiB)
		// |  |  |
		// |  |  |- TaskMemoryDescRange
		// |  |  |    - task memory descriptors
		// |  |  |
		// |  |  |- Guard (2 MiB)
		// |  |  |
		// |  |  |- TaskMemoryHeaderRange
		// |  |  |    - task memory ownership metadata
		// |  |  |
		// |  |  |- Guard (2 MiB)
		// |  |  |
		// |  |  |- TaskContextRange
		// |  |  |    - runtime execution context
		// |  |  |
		// |  |  |- Guard (2 MiB)
		// |  |  |
		// |  |  |- TaskSliceContextRange
		// |  |  |    - slice execution state
		// |  |  |
		// |  |  |- Guard (2 MiB)
		// |  |  |
		// |  |  |- GPUContextRange
		// |  |  |    - GPU execution state
		// |  |  |
		// |  |  |- Guard (2 MiB)
		// |  |  |
		// |  |  |- ObjectLocationSpare
		// |  |
		// |  |- Guard (2 MiB)
		// |  |
		// |  |- Input Layouts (3 GiB)
		// |  |  |
		// |  |  |- InputSizeArrays
		// |  |  |    - input size metadata
		// |  |  |
		// |  |  |- Guard (2 MiB)
		// |  |  |
		// |  |  |- InputAlignmentArrays
		// |  |  |    - input alignment metadata
		// |  |  |
		// |  |  |- Guard (2 MiB)
		// |  |  |
		// |  |  |- InputLayoutSpare
		// |  |
		// |  |- Guard (2 MiB)
		// |  |
		// |  |- Output Layouts
		// |     |
		// |     |- OutputSizeArrays
		// |     |    - output size metadata
		// |     |
		// |     |- Guard (2 MiB)
		// |     |
		// |     |- OutputAlignmentArrays
		// |     |    - output alignment metadata
		// |     |
		// |     |- Guard (2 MiB)
		// |     |
		// |     |- OutputLayoutSpare
		// |
		// |- TASK METADATA GUARD (2 MiB)
		// |
		// |- TASK PAYLOAD VA (~14 GiB)
		// |  |
		// |  |- TaskPayloadArena
		// |       - task payload buffers
		// |       - input/output data
		// |       - reductions
		// |       - GPU-visible payload
		// |
		// |- TASK PAYLOAD GUARD (2 MiB)
		// |
		// |- RESERVED / FUTURE VA (~4 GiB)
		// |  - GPU staging / DMA
		// |  - NUMA mirroring
		// |  - sanitizer / shadow memory
		// |  - RDMA / remote memory
		// |
		// |- LOWER NULL GUARD (2 MiB)
		//
		// ==============================================================================
		// Design Notes
		// ------------------------------------------------------------------------------
		// - Entire runtime lives in a single reserved VA block.
		// - Subsystems are partitioned into deterministic regions.
		// - 2 MiB guard regions detect linear memory overruns.
		// - Task metadata uses structure-of-arrays layout for cache efficiency.
		// - Payload memory is isolated from metadata to prevent corruption.
		// - All regions are sliced once at startup and never moved.
		// ==============================================================================

		g_GlobalMemoryHeaderMemory = VirtualMemory::virtualAlloc(
			g_GlobalMemoryHeaderMemory,
			g_TotalVA,
			MemoryOperation::Reserve
		);

		g_GlobalVA = VARegionSlicer{ g_GlobalMemoryHeaderMemory };

		// --- Top-level VA ----------------------------------------------------

		g_UpperNullGuard = g_GlobalVA.slice(NullGuardSize);

		g_RuntimeVA = g_GlobalVA.slice(RuntimeVASize);
		g_RuntimeGuard = g_GlobalVA.slice(SectionGuardSize);

		g_TaskMetadataVA = g_GlobalVA.slice(TaskMetadataVASize);
		g_TaskMetadataGuard = g_GlobalVA.slice(SectionGuardSize);

		g_TaskPayloadVA = g_GlobalVA.slice(TaskPayloadVASize);
		g_TaskPayloadGuard = g_GlobalVA.slice(SectionGuardSize);

		g_ReservedVA = g_GlobalVA.slice(ReservedVASize);
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

			g_TaskMemoryDescRange = obj.slice(Bytes{ MaxTasks * sizeof(TaskMemoryDescHeader) });
			g_TaskMemoryDescGuard = obj.slice(SectionGuardSize);

			g_TaskMemoryHeaderRange = obj.slice(Bytes{ MaxTasks * sizeof(TaskMemoryHeader) });
			g_TaskMemoryHeaderGuard = obj.slice(SectionGuardSize);

			g_TaskContextRange = obj.slice(Bytes{ MaxTasks * sizeof(TaskContextHeader) });
			g_TaskContextGuard = obj.slice(SectionGuardSize);

			g_TaskSliceContextRange = obj.slice(Bytes{ MaxTasks * sizeof(TaskSliceContextHeader) });
			g_TaskSliceContextGuard = obj.slice(SectionGuardSize);

			g_GPUContextRange = obj.slice(Bytes{ MaxTasks * sizeof(GPUContextHeader) });
			g_GPUContextGuard = obj.slice(SectionGuardSize);

			g_ObjectLocationSpare = obj.slice(obj.remaining());
		}

		// --- Input Layouts ---------------------------------------------------

		{
			VARegionSlicer in{ g_TaskInputLayouts };

			g_InputSizeArrays = in.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
			g_InputSizeGuard = in.slice(SectionGuardSize);

			g_InputAlignmentArrays = in.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
			g_InputAlignmentGuard = in.slice(SectionGuardSize);

			g_InputLayoutSpare = in.slice(in.remaining());
		}

		// --- Output Layouts --------------------------------------------------

		{
			VARegionSlicer out{ g_TaskOutputLayouts };

			g_OutputSizeArrays = out.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
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

		// --- Lock guard regions ----------------------------------------------

		lockGuard(g_UpperNullGuard);
		lockGuard(g_RuntimeGuard);
		lockGuard(g_TaskMetadataGuard);
		lockGuard(g_TaskPayloadGuard);
		lockGuard(g_LowerNullGuard);

		lockGuard(g_ClosureGuard);
		lockGuard(g_SmartPtrGuard);

		lockGuard(g_ObjectLocationsGuard);
		lockGuard(g_InputLayoutsGuard);

		lockGuard(g_TaskMemoryDescGuard);
		lockGuard(g_TaskMemoryHeaderGuard);
		lockGuard(g_TaskContextGuard);
		lockGuard(g_TaskSliceContextGuard);
		lockGuard(g_GPUContextGuard);

		lockGuard(g_InputSizeGuard);
		lockGuard(g_InputAlignmentGuard);

		lockGuard(g_OutputSizeGuard);
		lockGuard(g_OutputAlignmentGuard);

		return true;
	}

} // namespace Corium::Memory::Internal