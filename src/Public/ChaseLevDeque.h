#pragma once
#include "CoriumAllocator.h"
#include "CoriumMemory.h"

// TODO
namespace Corium::Runtime::WorkStealers {
	template<typename T>
	class ChaseLevAllocator final :  public Corium::Memory::Allocators::IArena<ChaseLevAllocator<T>> {
		Memory::VirtualSegment m_DequeRegion;
		static constexpr size_t s_TotalLength = 1024;

	public:
		ChaseLevAllocator() {
			using namespace Memory::Literals;
			m_DequeRegion = Memory::VirtualMemory::virtualAlloc(
				Memory::INVALID_SEGMENT,
				sizeof(T) * s_TotalLength + 2 * Memory::PAGE_SIZE,
				Memory::MemoryOperation::Reserve);

			Memory::Internal::VARegionSlicer slicer{ m_DequeRegion };
			CORIUM_UNUSED(Memory::VirtualMemory::protectMem(Memory::createSegment(slicer.slice(Memory::PAGE_SIZE))));
			m_DequeRegion = Memory::createSegment(slicer.slice(m_DequeRegion.m_TotalSize - 2 * Memory::PAGE_SIZE));
			CORIUM_UNUSED(Memory::VirtualMemory::protectMem(Memory::createSegment(slicer.slice(Memory::PAGE_SIZE))));
		}

		Memory::Allocators::AllocationHeader allocateImpl() {
			
		}

		bool deallocateImpl(Memory::Allocators::AllocationHeader& ro_Header) {
			
		}


	};
}
