#pragma once
#include "Engine/Memory/MovableMemory.hpp"

#include <stdlib.h>
#include <cstdio>
#include <new>


//----------------------------------------------------------------------------
template<typename LockPolicy, typename MovementPolicy>
requires Defragmentable<MovementPolicy>
MemoryManager<LockPolicy, MovementPolicy>::MemoryManager(byte* memBlockToManage, uint64_t memBlockSize)
{
	Initialize(memBlockToManage, memBlockSize);
}


//----------------------------------------------------------------------------
template<typename LockPolicy, typename MovementPolicy>
requires Defragmentable<MovementPolicy>
void MemoryManager<LockPolicy, MovementPolicy>::Initialize(byte* memBlockToManage, uint64_t memBlockSize)
{
	m_mainMemoryBlock = memBlockToManage;
	m_remainingMainMemory = memBlockSize;

	if (m_mainMemoryBlock == nullptr)
	{
		printf("Can't manage a null memory block");
		return;
	}

	// Initialize the first index entry
	m_memoryIndexHead = new(m_indexPool.Allocate()) IndexEntry(m_mainMemoryBlock, m_remainingMainMemory);
}


//----------------------------------------------------------------------------
template<typename LockPolicy, typename MovementPolicy>
requires Defragmentable<MovementPolicy>
MemoryManager<LockPolicy, MovementPolicy>::~MemoryManager()
{
	// Don't free m_mainMemoryBlock since MemoryManager doesn't own it
}


//----------------------------------------------------------------------------
template<typename LockPolicy, typename MovementPolicy>
requires Defragmentable<MovementPolicy>
void* MemoryManager<LockPolicy, MovementPolicy>::Allocate(int size, int count, int alignment)
{
	IndexEntry* newIndexEntry = AllocateNonMovable(size, count, alignment);
	if (newIndexEntry == nullptr)
	{
		return nullptr;
	}

	return newIndexEntry->dataLocation;
}


//----------------------------------------------------------------------------
// Alignment is what address it's aligned to so use a pointer to data and pointer to block start
template<typename LockPolicy, typename MovementPolicy>
requires Defragmentable<MovementPolicy>
IndexEntry* MemoryManager<LockPolicy, MovementPolicy>::AllocateNonMovable(int size, int count, int alignment)
{
	if (size <= 0 || count <= 0 || alignment <= 0)
	{
		if (size <= 0)		printf("Can't allocate memory with size less than 1 \n");
		if (count <= 0)		printf("Can't allocate memory with count less than 1 \n");
		if (alignment <= 0)	printf("Can't allocate memory with alignment less than 1 \n");
		
		return nullptr;
	}

	uint64_t totalSize = (uint64_t)(size * count);

	LockPolicy lock(m_mutex);
	if (m_remainingMainMemory < totalSize)
	{
		printf("Not enough total memory remaining to allocate new blocks. size: %d count:%d \n", size, count);
		return nullptr;
	}

	IndexEntry* curIndexEntry = m_memoryIndexHead;
	while (curIndexEntry != nullptr)
	{
		if (!curIndexEntry->isFree || curIndexEntry->size < totalSize)
		{
			curIndexEntry = curIndexEntry->nextEntry;
			continue;
		}

		// Find number of padding bytes needed for requested alignment
		uint64_t numPaddingBytes = (uint64_t)curIndexEntry->blockLocation % (uint64_t)alignment;
		if (numPaddingBytes > 0)
		{
			numPaddingBytes = alignment - numPaddingBytes;
		}
		totalSize += numPaddingBytes;

		// If this block isn't large enough after padding, skip it
		if (curIndexEntry->size < totalSize)
		{
			curIndexEntry = curIndexEntry->nextEntry;
			continue;
		}

		// Found a block exactly large enough, return it
		if (curIndexEntry->size == totalSize)
		{
			curIndexEntry->isFree = false;
			curIndexEntry->dataLocation = curIndexEntry->blockLocation + numPaddingBytes;
			curIndexEntry->isPinned = true;

			return curIndexEntry;
		}

		// Found a free block larger than requested, need to split it and return sub block
		// Update newly allocated entry index
		byte* blockLocation = curIndexEntry->blockLocation + totalSize;
		uint64_t size = curIndexEntry->size - totalSize;
		IndexEntry* nextEntry = new(m_indexPool.Allocate()) IndexEntry(blockLocation, size);
		
		// Chop memory from the front of memory pointed to by curIndexEntry to use as new memory block
		nextEntry->nextEntry = curIndexEntry->nextEntry;

		// Update existing entry index
		curIndexEntry->isFree = false;
		curIndexEntry->size = totalSize;
		curIndexEntry->dataLocation = curIndexEntry->blockLocation + numPaddingBytes;
		curIndexEntry->nextEntry = nextEntry;
		curIndexEntry->isPinned = true;

		m_remainingMainMemory -= totalSize;
		return curIndexEntry;
	}

	printf("Could not find a block large enough to meet request. size: %d count:%d \n", size, count);
	return nullptr;
}


// Realloc
// allocate, memcpy, free a large enough block
// or is one you have large enough find a large enough existing - find in list, if big enough (from padding) return
template<typename LockPolicy, typename MovementPolicy>
requires Defragmentable<MovementPolicy>
void* MemoryManager<LockPolicy, MovementPolicy>::Reallocate(void* ptrToMemory, uint64_t newSize)
{
	if (ptrToMemory == nullptr)
	{
		printf("Can't reallocate a nullptr\n");
		return ptrToMemory;
	}

	LockPolicy lock(m_mutex);

	// Find entry along with surrounding entries
	ReallocIndexEntrySearchResult searchResult;
	if (!FindIndexEntryFromDataPtrForRealloc(ptrToMemory, searchResult, newSize))
	{
		printf("Tried to realloc a ptr not associated with manager\n");
		return ptrToMemory;
	}

	// Handle cases where existing entry is large enough to accommodate realloc
	if (searchResult.entry == searchResult.targetEntry)
	{
		if (searchResult.entry->size >= newSize)
		{
			return ptrToMemory;
		}
	}
	
	// New size is larger than current
	if (searchResult.targetEntry == nullptr)
	{
		printf("No block large enough to fulfill realloc request\n");
		return ptrToMemory;
	}

	IndexEntry*& srcEntry = searchResult.entry;
	IndexEntry*& destEntry = searchResult.targetEntry;

	uint64_t dataOffset = srcEntry->dataLocation - srcEntry->blockLocation;
	destEntry->dataLocation = destEntry->blockLocation + dataOffset;

	// Copy the actual data
	memmove(destEntry->dataLocation, srcEntry->dataLocation, srcEntry->size - dataOffset);

	// Update index entries
	uint64_t sizeDifference = destEntry->size - newSize;
	destEntry->size = newSize;
	destEntry->isFree = false;
	destEntry->isPinned = srcEntry->isPinned;
	
	if (sizeDifference != 0)
	{
		// Add new entry for remainder
		IndexEntry* newNextEntry = new(m_indexPool.Allocate()) IndexEntry(destEntry->blockLocation + newSize, sizeDifference);
		newNextEntry->nextEntry = destEntry->nextEntry;
		
		destEntry->nextEntry = newNextEntry;
	}

	// Update total remaining memory to reflect this realloc
	uint64_t oldSize = srcEntry->size;
	m_remainingMainMemory -= newSize - oldSize;

	// Free old entry
	FreeAndCoalesceIndexEntry(searchResult.entry, searchResult.prevEntry);

	return ptrToMemory;
}


//----------------------------------------------------------------------------
template<typename LockPolicy, typename MovementPolicy>
requires Defragmentable<MovementPolicy>
void MemoryManager<LockPolicy, MovementPolicy>::Free(void* ptrToMemory)
{
	LockPolicy lock(m_mutex);

	IndexEntry* indexEntryToFree = nullptr;
	IndexEntry* indexEntryToFreePrev = nullptr;
	if (!FindIndexEntryFromDataPtr(ptrToMemory, indexEntryToFree, indexEntryToFreePrev))
	{
		printf("Tried to free a ptr not associated with manager\n");
		return;
	}

	m_remainingMainMemory += indexEntryToFree->size;
	FreeAndCoalesceIndexEntry(indexEntryToFree, indexEntryToFreePrev);
}


//---------------------------------------------------------------------------
template<typename LockPolicy, typename MovementPolicy>
requires Defragmentable<MovementPolicy>
void MemoryManager<LockPolicy, MovementPolicy>::DebugPrint()
{
	printf("\n---- Memory Layout ----\n");

	LockPolicy lock(m_mutex);
	IndexEntry* curIndexEntry = m_memoryIndexHead;
	while (curIndexEntry != nullptr)
	{
		printf("IndexEntry: %p Location: %p DataLocation: %p  Size: %lld  Free: %i Pinned: %i\n", curIndexEntry, curIndexEntry->blockLocation, curIndexEntry->dataLocation, curIndexEntry->size, curIndexEntry->isFree, curIndexEntry->isPinned);

		curIndexEntry = curIndexEntry->nextEntry;
	}
}
