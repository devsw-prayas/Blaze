#include "Corium.h"
#include "CoriumMemoryHandler.h"

namespace Corium::Memory::Internal {

	bool AllocatorRegistry::isRegistered = false;

	VirtualSegment AllocatorRegistry::s_ClosureMemory{};
	Allocators::ClosureAllocator AllocatorRegistry::s_ClosureAllocator{};

	VirtualSegment AllocatorRegistry::s_SmartPtrControlBlockMemory{};
	Allocators::ControlBlockAllocator AllocatorRegistry::s_ControlBlockAllocator{};

	VirtualSegment AllocatorRegistry::s_TaskMemoryDescMemory{};
	Allocators::TaskMetadataAllocator AllocatorRegistry::s_TaskMemoryDescAllocator{};

	VirtualSegment AllocatorRegistry::s_TaskMemoryHeaderMemory{};
	Allocators::TaskMetadataAllocator AllocatorRegistry::s_TaskMemoryHeaderAllocator{};

	VirtualSegment AllocatorRegistry::s_TaskContextMemory{};
	Allocators::TaskMetadataAllocator AllocatorRegistry::s_TaskContextAllocator{};

	VirtualSegment AllocatorRegistry::s_TaskSliceContextMemory{};
	Allocators::TaskMetadataAllocator AllocatorRegistry::s_TaskSliceContextAllocator{};

	VirtualSegment AllocatorRegistry::s_GPUContextMemory{};
	Allocators::TaskMetadataAllocator AllocatorRegistry::s_GPUContextAllocator{};

	VirtualSegment AllocatorRegistry::s_InputSizeMemory{};
	Allocators::TaskMetadataAllocator AllocatorRegistry::s_InputSizeAllocator{};

	VirtualSegment AllocatorRegistry::s_InputAlignmentMemory{};
	Allocators::TaskMetadataAllocator AllocatorRegistry::s_InputAlignmentAllocator{};

	VirtualSegment AllocatorRegistry::s_OutputSizeMemory{};
	Allocators::TaskMetadataAllocator AllocatorRegistry::s_OutputSizeAllocator{};

	VirtualSegment AllocatorRegistry::s_OutputAlignmentMemory{};
	Allocators::TaskMetadataAllocator AllocatorRegistry::s_OutputAlignmentAllocator{};

	VirtualSegment AllocatorRegistry::s_TaskPayloadMemory{};
	Allocators::TaskPayloadAllocator AllocatorRegistry::s_TaskPayloadAllocator{};

	AtomicAllocators& AtomicAllocators::instance() {
		static AtomicAllocators inst;
		return inst;
	}
}
