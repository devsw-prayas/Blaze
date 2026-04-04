	#include "Corium.h"
#include "CoriumAddrSpace.h"

namespace Corium::Memory::Internal {

	// -------------------------------------------------------------------------
	// Per-node base reservations
	// -------------------------------------------------------------------------

	VirtualSegment g_NodeMemory[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// Top-level VA regions
	// -------------------------------------------------------------------------

	VARegion g_UpperNullGuard[MAX_NUMA_NODES];
	VARegion g_RuntimeVA[MAX_NUMA_NODES];
	VARegion g_RuntimeGuard[MAX_NUMA_NODES];
	VARegion g_TaskMetadataVA[MAX_NUMA_NODES];
	VARegion g_TaskMetadataGuard[MAX_NUMA_NODES];
	VARegion g_TaskPayloadVA[MAX_NUMA_NODES];
	VARegion g_TaskPayloadGuard[MAX_NUMA_NODES];
	VARegion g_ReservedVA[MAX_NUMA_NODES];
	VARegion g_LowerNullGuard[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// Runtime / Infra VA sub-regions
	// -------------------------------------------------------------------------

	VARegion g_ClosureRange[MAX_NUMA_NODES];
	VARegion g_ClosureGuard[MAX_NUMA_NODES];
	VARegion g_SmartPtrControlBlocks[MAX_NUMA_NODES];
	VARegion g_SmartPtrGuard[MAX_NUMA_NODES];
	VARegion g_RuntimeCoreObjects[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Level 1 sections
	// -------------------------------------------------------------------------

	VARegion g_TaskObjectLocations[MAX_NUMA_NODES];
	VARegion g_ObjectLocationsGuard[MAX_NUMA_NODES];

	VARegion g_TaskInputLayouts[MAX_NUMA_NODES];
	VARegion g_InputLayoutsGuard[MAX_NUMA_NODES];

	VARegion g_TaskOutputLayouts[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Object Locations (typed ranges)
	// -------------------------------------------------------------------------

	VARegion g_TaskMemoryDescRange[MAX_NUMA_NODES];
	VARegion g_TaskMemoryDescGuard[MAX_NUMA_NODES];

	VARegion g_TaskMemoryHeaderRange[MAX_NUMA_NODES];
	VARegion g_TaskMemoryHeaderGuard[MAX_NUMA_NODES];

	VARegion g_TaskContextRange[MAX_NUMA_NODES];
	VARegion g_TaskContextGuard[MAX_NUMA_NODES];

	VARegion g_TaskSliceContextRange[MAX_NUMA_NODES];
	VARegion g_TaskSliceContextGuard[MAX_NUMA_NODES];

	VARegion g_GPUContextRange[MAX_NUMA_NODES];
	VARegion g_GPUContextGuard[MAX_NUMA_NODES];

	VARegion g_ObjectLocationSpare[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Input Layouts (typed ranges)
	// -------------------------------------------------------------------------

	VARegion g_InputSizeArrays[MAX_NUMA_NODES];
	VARegion g_InputSizeGuard[MAX_NUMA_NODES];

	VARegion g_InputAlignmentArrays[MAX_NUMA_NODES];
	VARegion g_InputAlignmentGuard[MAX_NUMA_NODES];

	VARegion g_InputLayoutSpare[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// TaskMetadata VA - Output Layouts (typed ranges)
	// -------------------------------------------------------------------------

	VARegion g_OutputSizeArrays[MAX_NUMA_NODES];
	VARegion g_OutputSizeGuard[MAX_NUMA_NODES];

	VARegion g_OutputAlignmentArrays[MAX_NUMA_NODES];
	VARegion g_OutputAlignmentGuard[MAX_NUMA_NODES];

	VARegion g_OutputLayoutSpare[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// Task Payload VA
	// -------------------------------------------------------------------------

	VARegion g_TaskPayloadArena[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// init()
	// -------------------------------------------------------------------------

	bool init() {
		// ==============================================================================
		//                  CORIUM VIRTUAL ADDRESS SPACE HIERARCHY
		//                  512 GiB total — 4 NUMA nodes × 128 GiB per node
		// ==============================================================================
		//
		// Each node receives a fully independent 128 GiB VA reservation via
		// VirtualAllocExNuma, so physical pages committed later remain local
		// to that node. The identical region layout is replicated per node.
		//
		// Per-node layout:
		//
		// NODE[n] VA (128 GiB reserved)
		// |
		// |  UPPER NULL GUARD (2 MiB)
		// |
		// |- RUNTIME / INFRASTRUCTURE VA (~24 GiB)
		// |  |
		// |  |- ClosureRange
		// |  |    - ClosureFunction objects
		// |  |    - 3 closures per task (startup / body / shutdown)
		// |  |
		// |  |- Guard (2 MiB)
		// |  |
		// |  |- SmartPtrControlBlocks (8 GiB)
		// |  |    - shared_ptr / intrusive control blocks
		// |  |
		// |  |- Guard (2 MiB)
		// |  |
		// |  |- RuntimeCoreObjects (remaining ~14 GiB)
		// |       - schedulers, executors, pools, global allocators
		// |
		// |- RUNTIME GUARD (2 MiB)
		// |
		// |- TASK METADATA VA (~32 GiB)
		// |  |
		// |  |- Object Locations (12 GiB)
		// |  |  |- TaskMemoryDescRange, TaskMemoryHeaderRange
		// |  |  |- TaskContextRange, TaskSliceContextRange, GPUContextRange
		// |  |  |- Guards (2 MiB each) + ObjectLocationSpare
		// |  |
		// |  |- Guard (2 MiB)
		// |  |
		// |  |- Input Layouts (12 GiB)
		// |  |  |- InputSizeArrays, InputAlignmentArrays
		// |  |  |- Guards (2 MiB each) + InputLayoutSpare
		// |  |
		// |  |- Guard (2 MiB)
		// |  |
		// |  |- Output Layouts (remaining ~8 GiB)
		// |     |- OutputSizeArrays, OutputAlignmentArrays
		// |     |- Guards (2 MiB each) + OutputLayoutSpare
		// |
		// |- TASK METADATA GUARD (2 MiB)
		// |
		// |- TASK PAYLOAD VA (~56 GiB)
		// |  |- TaskPayloadArena
		// |       - task payload buffers, input/output data,
		// |         reductions, GPU-visible payload
		// |
		// |- TASK PAYLOAD GUARD (2 MiB)
		// |
		// |- RESERVED / FUTURE VA (~16 GiB)
		// |  - GPU staging / DMA, NUMA-local staging,
		// |    sanitizer memory, RDMA
		// |
		// |- LOWER NULL GUARD (2 MiB)
		//
		// ==============================================================================

		for (uint32_t node = 0; node < MAX_NUMA_NODES; ++node) {
			// Reserve 128 GiB for this NUMA node.
			// Setting m_NumaNode on the input segment signals VirtualAllocExNuma.
			VirtualSegment req{};
			req.m_NumaNode = static_cast<uint8_t>(node);

			g_NodeMemory[node] = VirtualMemory::virtualAlloc(req, NodeVASize, MemoryOperation::Reserve);

			VARegionSlicer slicer{ g_NodeMemory[node] };

			// --- Top-level VA ----------------------------------------------------

			g_UpperNullGuard[node]    = slicer.slice(NullGuardSize);

			g_RuntimeVA[node]         = slicer.slice(RuntimeVASize);
			g_RuntimeGuard[node]      = slicer.slice(SectionGuardSize);

			g_TaskMetadataVA[node]    = slicer.slice(TaskMetadataVASize);
			g_TaskMetadataGuard[node] = slicer.slice(SectionGuardSize);

			g_TaskPayloadVA[node]     = slicer.slice(TaskPayloadVASize);
			g_TaskPayloadGuard[node]  = slicer.slice(SectionGuardSize);

			g_ReservedVA[node]        = slicer.slice(NodeReservedVASize);
			g_LowerNullGuard[node]    = slicer.slice(NullGuardSize);

			// --- Runtime VA ------------------------------------------------------

			{
				VARegionSlicer rt{ g_RuntimeVA[node] };

				// 3 closures per task × 256 bytes each
				g_ClosureRange[node]          = rt.slice(Bytes{ MaxTasks * 3 * 256 });
				g_ClosureGuard[node]          = rt.slice(SectionGuardSize);

				// 8 GiB for smart pointer control blocks (4× the original 2 GiB)
				g_SmartPtrControlBlocks[node] = rt.slice(Bytes{ 8_GiB });
				g_SmartPtrGuard[node]         = rt.slice(SectionGuardSize);

				// Remaining VA for schedulers, executors, pools, global allocators
				g_RuntimeCoreObjects[node]    = rt.slice(rt.remaining());
			}

			// --- TaskMetadata VA -------------------------------------------------

			{
				VARegionSlicer meta{ g_TaskMetadataVA[node] };

				// 12 GiB per section (4× the original 3 GiB)
				g_TaskObjectLocations[node]  = meta.slice(Bytes{ 12_GiB });
				g_ObjectLocationsGuard[node] = meta.slice(SectionGuardSize);

				g_TaskInputLayouts[node]     = meta.slice(Bytes{ 12_GiB });
				g_InputLayoutsGuard[node]    = meta.slice(SectionGuardSize);

				// Remaining ~8 GiB goes to output layouts
				g_TaskOutputLayouts[node]    = meta.slice(meta.remaining());
			}

			// --- Object Locations ------------------------------------------------

			{
				VARegionSlicer obj{ g_TaskObjectLocations[node] };

				g_TaskMemoryDescRange[node]   = obj.slice(Bytes{ MaxTasks * sizeof(TaskMemoryDescHeader) });
				g_TaskMemoryDescGuard[node]   = obj.slice(SectionGuardSize);

				g_TaskMemoryHeaderRange[node] = obj.slice(Bytes{ MaxTasks * sizeof(TaskMemoryHeader) });
				g_TaskMemoryHeaderGuard[node] = obj.slice(SectionGuardSize);

				g_TaskContextRange[node]      = obj.slice(Bytes{ MaxTasks * sizeof(TaskContextHeader) });
				g_TaskContextGuard[node]      = obj.slice(SectionGuardSize);

				g_TaskSliceContextRange[node] = obj.slice(Bytes{ MaxTasks * sizeof(TaskSliceContextHeader) });
				g_TaskSliceContextGuard[node] = obj.slice(SectionGuardSize);

				g_GPUContextRange[node]       = obj.slice(Bytes{ MaxTasks * sizeof(GPUContextHeader) });
				g_GPUContextGuard[node]       = obj.slice(SectionGuardSize);

				g_ObjectLocationSpare[node]   = obj.slice(obj.remaining());
			}

			// --- Input Layouts ---------------------------------------------------

			{
				VARegionSlicer in{ g_TaskInputLayouts[node] };

				g_InputSizeArrays[node]      = in.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
				g_InputSizeGuard[node]       = in.slice(SectionGuardSize);

				g_InputAlignmentArrays[node] = in.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
				g_InputAlignmentGuard[node]  = in.slice(SectionGuardSize);

				g_InputLayoutSpare[node]     = in.slice(in.remaining());
			}

			// --- Output Layouts --------------------------------------------------

			{
				VARegionSlicer out{ g_TaskOutputLayouts[node] };

				g_OutputSizeArrays[node]      = out.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
				g_OutputSizeGuard[node]       = out.slice(SectionGuardSize);

				g_OutputAlignmentArrays[node] = out.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
				g_OutputAlignmentGuard[node]  = out.slice(SectionGuardSize);

				g_OutputLayoutSpare[node]     = out.slice(out.remaining());
			}

			// --- Payload VA ------------------------------------------------------

			{
				VARegionSlicer payload{ g_TaskPayloadVA[node] };
				g_TaskPayloadArena[node] = payload.slice(payload.remaining());
			}

			// --- Lock guard regions for this node --------------------------------

			lockGuard(g_UpperNullGuard[node]);
			lockGuard(g_RuntimeGuard[node]);
			lockGuard(g_TaskMetadataGuard[node]);
			lockGuard(g_TaskPayloadGuard[node]);
			lockGuard(g_LowerNullGuard[node]);

			lockGuard(g_ClosureGuard[node]);
			lockGuard(g_SmartPtrGuard[node]);

			lockGuard(g_ObjectLocationsGuard[node]);
			lockGuard(g_InputLayoutsGuard[node]);

			lockGuard(g_TaskMemoryDescGuard[node]);
			lockGuard(g_TaskMemoryHeaderGuard[node]);
			lockGuard(g_TaskContextGuard[node]);
			lockGuard(g_TaskSliceContextGuard[node]);
			lockGuard(g_GPUContextGuard[node]);

			lockGuard(g_InputSizeGuard[node]);
			lockGuard(g_InputAlignmentGuard[node]);

			lockGuard(g_OutputSizeGuard[node]);
			lockGuard(g_OutputAlignmentGuard[node]);
		}

		return true;
	}

} // namespace Corium::Memory::Internal
