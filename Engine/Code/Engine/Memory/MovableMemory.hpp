#pragma once
#include "Engine/Memory/IndexEntry.hpp"


//----------------------------------------------------------------------------
template <typename T> 
concept Defragmentable = requires(T value, IndexEntry*& memMgrHead, IndexEntryObjectPool* indexEntryPool)
{
	{ value.Defragment(memMgrHead, indexEntryPool) } -> std::same_as<void>;
};


//----------------------------------------------------------------------------
class RawMemory
{
public:
	static void Defragment(IndexEntry*& memMgrHead, IndexEntryObjectPool* indexEntryPool) 
	{ 
		// Null op for raw memory 
		(void)memMgrHead; (void)indexEntryPool;
	}
};


//----------------------------------------------------------------------------
class MovableMemory
{
public:
	// Iterate over each index entry
	// 1. Pinned - skip
	// 2. In use - skip, unless we've seen a free block, then swap
	// 3. Free - save as freeBlockToSwap if we don't have one, coallesce with freeBlockToSwap otherwise
	// Note: Pass head by value in case it is swapped during defragmentation
	static void Defragment(IndexEntry*& memMgrHead, IndexEntryObjectPool* indexEntryPool)
	{
		IndexEntry* curIndexEntry = memMgrHead;
		IndexEntry* freeBlockToSwap = nullptr;
		IndexEntry* prevIndexEntry = nullptr;

		while (curIndexEntry != nullptr)
		{
			if (!curIndexEntry->isFree)
			{
				// If the block is pinned, skip it and reset the search for a free block
				if (curIndexEntry->isPinned)
				{
					freeBlockToSwap = nullptr;
					prevIndexEntry = curIndexEntry;
					curIndexEntry = curIndexEntry->nextEntry;
					continue;
				}

				if (freeBlockToSwap != nullptr)
				{
					//byte* blockLocation = nullptr;
					//byte* dataLocation = nullptr;
					//IndexEntry* nextEntry = nullptr;
					//uint64_t size = 0xDEADBEEF;
					//int refCount = 1;
					//bool isPinned = false;
					//bool isFree = true;
				
					// Swap data of this block and the previous free block 
					
					// Move data into freeBlockToSwap
					IndexEntry*& srcEntry = curIndexEntry;
					IndexEntry*& destEntry = freeBlockToSwap;
					
					uint64_t dataOffset = srcEntry->dataLocation - srcEntry->blockLocation;
					destEntry->dataLocation = destEntry->blockLocation + dataOffset;

					memmove(destEntry->dataLocation, srcEntry->dataLocation, srcEntry->size - dataOffset);

					// Swap positions of index data in list to maintain ordering of headers and underlying memory
					
					// In the case where the first block is free, update head to be the block swapped into it's place to avoid losing the list
					if (prevIndexEntry == nullptr)
					{
						memMgrHead = curIndexEntry;
					}
					else
					{
						prevIndexEntry->nextEntry = curIndexEntry;
					}
					freeBlockToSwap->nextEntry = curIndexEntry->nextEntry;
					curIndexEntry->nextEntry = freeBlockToSwap;
					
					// Update new data index entry										
					// The locations were updated during the data copy, so just need to clone to our new data entry
					curIndexEntry->blockLocation = freeBlockToSwap->blockLocation;
					curIndexEntry->dataLocation = freeBlockToSwap->dataLocation;

					// Update new free index entry
					freeBlockToSwap->blockLocation = curIndexEntry->blockLocation + curIndexEntry->size;
					freeBlockToSwap->dataLocation = freeBlockToSwap->blockLocation;

					// Advance pointers for next iteration of search
					prevIndexEntry = curIndexEntry;
					//freeBlockToSwap stays the same
					curIndexEntry = freeBlockToSwap->nextEntry;
					continue;
				}

				prevIndexEntry = curIndexEntry;
				curIndexEntry = curIndexEntry->nextEntry;
			}
			// This block is free
			else
			{
				if (freeBlockToSwap != nullptr)
				{
					// Coallesce curIndexEntry into freeBlockToSwap
					freeBlockToSwap->size += curIndexEntry->size;
					freeBlockToSwap->nextEntry = curIndexEntry->nextEntry;
					indexEntryPool->Free(curIndexEntry);

					curIndexEntry = freeBlockToSwap->nextEntry;
					continue;
				}

				freeBlockToSwap = curIndexEntry;
				curIndexEntry = curIndexEntry->nextEntry;
			}
		}
	}
};
