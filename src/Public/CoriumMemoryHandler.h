#pragma once
#include "CoriumAddrSpace.h"
#include "EngineAllocators.h"

namespace Corium::Memory::Internal {
	struct CORIUM_RUNTIME_API AllocatorRegistry final {
		~AllocatorRegistry() = delete;
		AllocatorRegistry(const AllocatorRegistry&) = delete;
		AllocatorRegistry(AllocatorRegistry&&) noexcept = delete;

		AllocatorRegistry& operator=(const AllocatorRegistry&) = delete;
		AllocatorRegistry& operator=(AllocatorRegistry&&) noexcept = delete;
		AllocatorRegistry() = delete;

		static bool isRegistered;

		// ------------------------------------------------------------------------
		// Runtime / Infrastructure
		// ------------------------------------------------------------------------

		static VirtualSegment s_ClosureMemory;
		static Allocators::ClosureAllocator s_ClosureAllocator;

		static VirtualSegment s_SmartPtrControlBlockMemory;
		static Allocators::ControlBlockAllocator s_ControlBlockAllocator;

		// ------------------------------------------------------------------------
		// Task Metadata - Object Locations
		// ------------------------------------------------------------------------

		static VirtualSegment s_TaskMemoryDescMemory;
		static Allocators::TaskMetadataAllocator s_TaskMemoryDescAllocator;

		static VirtualSegment s_TaskMemoryHeaderMemory;
		static Allocators::TaskMetadataAllocator s_TaskMemoryHeaderAllocator;

		static VirtualSegment s_TaskContextMemory;
		static Allocators::TaskMetadataAllocator s_TaskContextAllocator;

		static VirtualSegment s_TaskSliceContextMemory;
		static Allocators::TaskMetadataAllocator s_TaskSliceContextAllocator;

		static VirtualSegment s_GPUContextMemory;
		static Allocators::TaskMetadataAllocator s_GPUContextAllocator;

		// ------------------------------------------------------------------------
		// Task Metadata - Input Layouts
		// ------------------------------------------------------------------------

		static VirtualSegment s_InputSizeMemory;
		static Allocators::TaskMetadataAllocator s_InputSizeAllocator;

		static VirtualSegment s_InputAlignmentMemory;
		static Allocators::TaskMetadataAllocator s_InputAlignmentAllocator;

		// ------------------------------------------------------------------------
		// Task Metadata - Output Layouts
		// ------------------------------------------------------------------------

		static VirtualSegment s_OutputSizeMemory;
		static Allocators::TaskMetadataAllocator s_OutputSizeAllocator;

		static VirtualSegment s_OutputAlignmentMemory;
		static Allocators::TaskMetadataAllocator s_OutputAlignmentAllocator;

		// ------------------------------------------------------------------------
		// Task Payload
		// ------------------------------------------------------------------------

		static VirtualSegment s_TaskPayloadMemory;
		static Allocators::TaskPayloadAllocator s_TaskPayloadAllocator;

		static bool initRegistry() {
			if (isRegistered)
				return false;

			s_ClosureMemory = createSegment(g_ClosureRange);
			s_ClosureAllocator.init(s_ClosureMemory);

			s_SmartPtrControlBlockMemory = createSegment(g_SmartPtrControlBlocks);
			s_ControlBlockAllocator.init(s_SmartPtrControlBlockMemory);

			s_TaskMemoryDescMemory = createSegment(g_TaskMemoryDescRange);
			s_TaskMemoryDescAllocator.init(s_TaskMemoryDescMemory);

			s_TaskMemoryHeaderMemory = createSegment(g_TaskMemoryHeaderRange);
			s_TaskMemoryHeaderAllocator.init(s_TaskMemoryHeaderMemory);

			s_TaskContextMemory = createSegment(g_TaskContextRange);
			s_TaskContextAllocator.init(s_TaskContextMemory);

			s_TaskSliceContextMemory = createSegment(g_TaskSliceContextRange);
			s_TaskSliceContextAllocator.init(s_TaskSliceContextMemory);

			s_GPUContextMemory = createSegment(g_GPUContextRange);
			s_GPUContextAllocator.init(s_GPUContextMemory);

			s_InputSizeMemory = createSegment(g_InputSizeArrays);
			s_InputSizeAllocator.init(s_InputSizeMemory);

			s_InputAlignmentMemory = createSegment(g_InputAlignmentArrays);
			s_InputAlignmentAllocator.init(s_InputAlignmentMemory);

			s_OutputSizeMemory = createSegment(g_OutputSizeArrays);
			s_OutputSizeAllocator.init(s_OutputSizeMemory);

			s_OutputAlignmentMemory = createSegment(g_OutputAlignmentArrays);
			s_OutputAlignmentAllocator.init(s_OutputAlignmentMemory);

			s_TaskPayloadMemory = createSegment(g_TaskPayloadArena);
			s_TaskPayloadAllocator.init(s_TaskPayloadMemory);

			isRegistered = true;
			return true;
		}
	};

	struct CORIUM_RUNTIME_API AtomicAllocators final {
		AtomicAllocators(const AtomicAllocators&) = delete;
		AtomicAllocators(AtomicAllocators&&) noexcept = delete;

		AtomicAllocators& operator=(const AtomicAllocators&) = delete;
		AtomicAllocators& operator=(AtomicAllocators&&) noexcept = delete;
		// ---------------------------------------------------------------------
		// Runtime / Infrastructure
		// ---------------------------------------------------------------------

		const Core::Atomic::AtomicPointer<Allocators::ClosureAllocator> s_ClosureAllocator;
		const Core::Atomic::AtomicPointer<Allocators::ControlBlockAllocator> s_ControlBlockAllocator;

		// ---------------------------------------------------------------------
		// Task Metadata - Object Locations
		// ---------------------------------------------------------------------

		const Core::Atomic::AtomicPointer<Allocators::TaskMetadataAllocator> s_TaskMemoryDescAllocator;
		const Core::Atomic::AtomicPointer<Allocators::TaskMetadataAllocator> s_TaskMemoryHeaderAllocator;
		const Core::Atomic::AtomicPointer<Allocators::TaskMetadataAllocator> s_TaskContextAllocator;
		const Core::Atomic::AtomicPointer<Allocators::TaskMetadataAllocator> s_TaskSliceContextAllocator;
		const Core::Atomic::AtomicPointer<Allocators::TaskMetadataAllocator> s_GPUContextAllocator;

		// ---------------------------------------------------------------------
		// Task Metadata - Input Layouts
		// ---------------------------------------------------------------------

		const Core::Atomic::AtomicPointer<Allocators::TaskMetadataAllocator> s_InputSizeAllocator;
		const Core::Atomic::AtomicPointer<Allocators::TaskMetadataAllocator> s_InputAlignmentAllocator;

		// ---------------------------------------------------------------------
		// Task Metadata - Output Layouts
		// ---------------------------------------------------------------------

		const Core::Atomic::AtomicPointer<Allocators::TaskMetadataAllocator> s_OutputSizeAllocator;
		const Core::Atomic::AtomicPointer<Allocators::TaskMetadataAllocator> s_OutputAlignmentAllocator;

		// ---------------------------------------------------------------------
		// Task Payload
		// ---------------------------------------------------------------------

		const Core::Atomic::AtomicPointer<Allocators::TaskPayloadAllocator> s_TaskPayloadAllocator;

	private:
		AtomicAllocators()
			: s_ClosureAllocator(&AllocatorRegistry::s_ClosureAllocator)
			, s_ControlBlockAllocator(&AllocatorRegistry::s_ControlBlockAllocator)

			, s_TaskMemoryDescAllocator(&AllocatorRegistry::s_TaskMemoryDescAllocator)
			, s_TaskMemoryHeaderAllocator(&AllocatorRegistry::s_TaskMemoryHeaderAllocator)
			, s_TaskContextAllocator(&AllocatorRegistry::s_TaskContextAllocator)
			, s_TaskSliceContextAllocator(&AllocatorRegistry::s_TaskSliceContextAllocator)
			, s_GPUContextAllocator(&AllocatorRegistry::s_GPUContextAllocator)

			, s_InputSizeAllocator(&AllocatorRegistry::s_InputSizeAllocator)
			, s_InputAlignmentAllocator(&AllocatorRegistry::s_InputAlignmentAllocator)

			, s_OutputSizeAllocator(&AllocatorRegistry::s_OutputSizeAllocator)
			, s_OutputAlignmentAllocator(&AllocatorRegistry::s_OutputAlignmentAllocator)

			, s_TaskPayloadAllocator(&AllocatorRegistry::s_TaskPayloadAllocator) {
		}
	public:
		static AtomicAllocators& instance();
	};
}
