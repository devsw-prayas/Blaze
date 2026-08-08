#include "Corium.h"
#include "CoriumAddrSpace.h"
#include "CoriumEnvironment.h"

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
	VARegion g_ThreadLocalStorage[MAX_NUMA_NODES];
	VARegion g_TLSGuard[MAX_NUMA_NODES];
	VARegion g_ReservedVA[MAX_NUMA_NODES];
	VARegion g_LowerNullGuard[MAX_NUMA_NODES];

	// -------------------------------------------------------------------------
	// Runtime / Infra VA sub-regions
	// -------------------------------------------------------------------------

	VARegion g_ClosureRange[MAX_NUMA_NODES];
	VARegion g_ClosureGuard[MAX_NUMA_NODES];
	VARegion g_SmartPtrControlBlocks[MAX_NUMA_NODES];
	VARegion g_SmartPtrGuard[MAX_NUMA_NODES];
	VARegion g_FrameStorage[MAX_NUMA_NODES];
	VARegion g_FrameStorageGuard[MAX_NUMA_NODES];
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
	// init() — carves g_NodeMemory[node] into the regions declared in
	// CoriumAddrSpace.h, in the same on-disk order as the box comments there.
	// -------------------------------------------------------------------------

	bool init(uint32_t v_NodeCount) {
		VirtualMemory::init();

		CORIUM_ASSERT(v_NodeCount <= MAX_NUMA_NODES);

		for (uint32_t node = 0; node < v_NodeCount; ++node) {
			// Reserve 128 GiB for this NUMA node.
			// Setting m_NumaNode on the input segment signals VirtualAllocExNuma.
			VirtualSegment req{};
			req.m_NumaNode = static_cast<uint8_t>(node);

			g_NodeMemory[node] = VirtualMemory::virtualAlloc(req, NodeVASize, MemoryOperation::Reserve);

			if (!g_NodeMemory[node].isValid())
				Environment::CoriumTermination::terminate("Failed to allocate memory, VirtualAlloc Failure",
					__FILE__, __LINE__);

			VARegionSlicer slicer{ g_NodeMemory[node] };

			// +----------------------------------------------------------------+
			// | UPPER NULL GUARD (2 MiB)                                       |
			g_UpperNullGuard[node] = slicer.slice(NullGuardSize);
			// +----------------------------------------------------------------+

			// +======================================================================+
			//   RUNTIME / INFRA VA  (~24 GiB)
			g_RuntimeVA[node] = slicer.slice(RuntimeVASize);
			// +======================================================================+

			// +----------------------------------------------------------------+
			// | RUNTIME GUARD (2 MiB)                                          |
			g_RuntimeGuard[node] = slicer.slice(SectionGuardSize);
			// +----------------------------------------------------------------+

			// +======================================================================+
			//   TASK METADATA VA  (~32 GiB)
			g_TaskMetadataVA[node] = slicer.slice(TaskMetadataVASize);
			// +======================================================================+

			// +----------------------------------------------------------------+
			// | TASK METADATA GUARD (2 MiB)                                    |
			g_TaskMetadataGuard[node] = slicer.slice(SectionGuardSize);
			// +----------------------------------------------------------------+

			// +======================================================================+
			//   TASK PAYLOAD VA  (~56 GiB)
			g_TaskPayloadVA[node] = slicer.slice(TaskPayloadVASize);
			// +======================================================================+

			// +----------------------------------------------------------------+
			// | TASK PAYLOAD GUARD (2 MiB)                                     |
			g_TaskPayloadGuard[node] = slicer.slice(SectionGuardSize);
			// +----------------------------------------------------------------+

			// +----------------------------------------------------------------+
			// | CORIUM THREAD LOCAL STORAGE (4 GiB)                            |
			g_ThreadLocalStorage[node] = slicer.slice(ThreadLocalStorageSize);
			// +----------------------------------------------------------------+

			// +----------------------------------------------------------------+
			// | TLS GUARD (2 MiB)                                              |
			g_TLSGuard[node] = slicer.slice(SectionGuardSize);
			// +----------------------------------------------------------------+

			// +----------------------------------------------------------------+
			// | RESERVED / FUTURE VA (~12 GiB)                                 |
			g_ReservedVA[node] = slicer.slice(NodeReservedVASize);
			// +----------------------------------------------------------------+

			// +----------------------------------------------------------------+
			// | LOWER NULL GUARD (2 MiB)                                       |
			g_LowerNullGuard[node] = slicer.slice(NullGuardSize);
			// +----------------------------------------------------------------+

			// --- Runtime VA -------------------------------------------------

			{
				VARegionSlicer rt{ g_RuntimeVA[node] };

				//   +------------------------------------------------------------+
				//   | ClosureRange — 3 ClosureFunctions/task x 256B each          |
				g_ClosureRange[node] = rt.slice(Bytes{ MaxTasks * 3 * 256 });
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | Guard (2 MiB)                                               |
				g_ClosureGuard[node] = rt.slice(SectionGuardSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | SmartPtrControlBlocks — 8 GiB                              |
				g_SmartPtrControlBlocks[node] = rt.slice(Bytes{ 8_GiB });
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | Guard (2 MiB)                                               |
				g_SmartPtrGuard[node] = rt.slice(SectionGuardSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | FrameStorage — 4 GiB, pool of 2048 x 2 MiB NativeFrame     |
				//   | blocks (header + inline execution stack per frame)        |
				g_FrameStorage[node] = rt.slice(FrameStorageSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | Guard (2 MiB)                                               |
				g_FrameStorageGuard[node] = rt.slice(SectionGuardSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | RuntimeCoreObjects — remaining ~10 GiB                     |
				g_RuntimeCoreObjects[node] = rt.slice(rt.remaining());
				//   +------------------------------------------------------------+
			}

			// --- TaskMetadata VA ---------------------------------------------

			{
				VARegionSlicer meta{ g_TaskMetadataVA[node] };

				//   +------------------------------------------------------------+
				//   | TaskObjectLocations — 12 GiB                                |
				g_TaskObjectLocations[node] = meta.slice(Bytes{ 12_GiB });
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | Guard (2 MiB)                                               |
				g_ObjectLocationsGuard[node] = meta.slice(SectionGuardSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | TaskInputLayouts — 12 GiB                                   |
				g_TaskInputLayouts[node] = meta.slice(Bytes{ 12_GiB });
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | Guard (2 MiB)                                               |
				g_InputLayoutsGuard[node] = meta.slice(SectionGuardSize);
				//   +------------------------------------------------------------+

				//   +------------------------------------------------------------+
				//   | TaskOutputLayouts — remaining ~8 GiB                        |
				g_TaskOutputLayouts[node] = meta.slice(meta.remaining());
				//   +------------------------------------------------------------+
			}

			// --- Object Locations ---------------------------------------------

			{
				VARegionSlicer obj{ g_TaskObjectLocations[node] };

				//     +----------------------------------------------------------+
				//     | TaskMemoryDescRange — MaxTasks x sizeof(TaskMemoryDescHeader) |
				g_TaskMemoryDescRange[node] = obj.slice(Bytes{ MaxTasks * sizeof(TaskMemoryDescHeader) });
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | Guard (2 MiB)                                             |
				g_TaskMemoryDescGuard[node] = obj.slice(SectionGuardSize);
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | TaskMemoryHeaderRange — MaxTasks x sizeof(TaskMemoryHeader) |
				g_TaskMemoryHeaderRange[node] = obj.slice(Bytes{ MaxTasks * sizeof(TaskMemoryHeader) });
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | Guard (2 MiB)                                             |
				g_TaskMemoryHeaderGuard[node] = obj.slice(SectionGuardSize);
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | TaskContextRange — MaxTasks x sizeof(TaskContextHeader)  |
				g_TaskContextRange[node] = obj.slice(Bytes{ MaxTasks * sizeof(TaskContextHeader) });
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | Guard (2 MiB)                                             |
				g_TaskContextGuard[node] = obj.slice(SectionGuardSize);
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | TaskSliceContextRange — MaxTasks x sizeof(TaskSliceContextHeader) |
				g_TaskSliceContextRange[node] = obj.slice(Bytes{ MaxTasks * sizeof(TaskSliceContextHeader) });
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | Guard (2 MiB)                                             |
				g_TaskSliceContextGuard[node] = obj.slice(SectionGuardSize);
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | GPUContextRange — MaxTasks x sizeof(GPUContextHeader)    |
				g_GPUContextRange[node] = obj.slice(Bytes{ MaxTasks * sizeof(GPUContextHeader) });
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | Guard (2 MiB)                                             |
				g_GPUContextGuard[node] = obj.slice(SectionGuardSize);
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | ObjectLocationSpare — remainder                          |
				g_ObjectLocationSpare[node] = obj.slice(obj.remaining());
				//     +----------------------------------------------------------+
			}

			// --- Input Layouts -------------------------------------------------

			{
				VARegionSlicer in{ g_TaskInputLayouts[node] };

				//     +----------------------------------------------------------+
				//     | InputSizeArrays — MaxTasks x ParamsPerTask x sizeof(size_t) |
				g_InputSizeArrays[node] = in.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | Guard (2 MiB)                                             |
				g_InputSizeGuard[node] = in.slice(SectionGuardSize);
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | InputAlignmentArrays — MaxTasks x ParamsPerTask x sizeof(size_t) |
				g_InputAlignmentArrays[node] = in.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | Guard (2 MiB)                                             |
				g_InputAlignmentGuard[node] = in.slice(SectionGuardSize);
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | InputLayoutSpare — remainder                             |
				g_InputLayoutSpare[node] = in.slice(in.remaining());
				//     +----------------------------------------------------------+
			}

			// --- Output Layouts ------------------------------------------------

			{
				VARegionSlicer out{ g_TaskOutputLayouts[node] };

				//     +----------------------------------------------------------+
				//     | OutputSizeArrays — MaxTasks x ParamsPerTask x sizeof(size_t) |
				g_OutputSizeArrays[node] = out.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | Guard (2 MiB)                                             |
				g_OutputSizeGuard[node] = out.slice(SectionGuardSize);
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | OutputAlignmentArrays — MaxTasks x ParamsPerTask x sizeof(size_t) |
				g_OutputAlignmentArrays[node] = out.slice(Bytes{ MaxTasks * ParamsPerTask * sizeof(size_t) });
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | Guard (2 MiB)                                             |
				g_OutputAlignmentGuard[node] = out.slice(SectionGuardSize);
				//     +----------------------------------------------------------+

				//     +----------------------------------------------------------+
				//     | OutputLayoutSpare — remainder                            |
				g_OutputLayoutSpare[node] = out.slice(out.remaining());
				//     +----------------------------------------------------------+
			}

			// --- Payload VA ------------------------------------------------------

			{
				VARegionSlicer payload{ g_TaskPayloadVA[node] };

				//   +------------------------------------------------------------+
				//   | TaskPayloadArena — entire TaskPayloadVA                    |
				g_TaskPayloadArena[node] = payload.slice(payload.remaining());
				//   +------------------------------------------------------------+
			}

			// --- Lock guard regions for this node --------------------------------

			lockGuard(g_UpperNullGuard[node]);
			lockGuard(g_RuntimeGuard[node]);
			lockGuard(g_TaskMetadataGuard[node]);
			lockGuard(g_TaskPayloadGuard[node]);
			lockGuard(g_TLSGuard[node]);
			lockGuard(g_LowerNullGuard[node]);

			lockGuard(g_ClosureGuard[node]);
			lockGuard(g_SmartPtrGuard[node]);
			lockGuard(g_FrameStorageGuard[node]);

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