#include "Engine/Memory/MemoryManager.hpp"


//----------------------------------------------------------------------------
bool BaseMemoryManager::FindIndexEntryFromDataPtrForRealloc(void* dataPtr, ReallocIndexEntrySearchResult& out_searchResult, uint64_t newSize)
{
	// TODO: Handle case where only block big enough would be the existing block coalesced with a neighbor
	// Try this first before search

	IndexEntry* curIndexEntry = m_memoryIndexHead;
	IndexEntry* prevIndexEntry = nullptr;
	bool foundDataPtr = false;

	while (curIndexEntry != nullptr)
	{
		if (curIndexEntry->dataLocation == dataPtr)
		{
			foundDataPtr = true;
			out_searchResult.entry = curIndexEntry;
			out_searchResult.prevEntry = prevIndexEntry;

			// Existing block can accommodate the realloc, return it as the target entry
			if (curIndexEntry->size >= newSize)
			{
				out_searchResult.targetEntry = curIndexEntry;
				return true;
			}
		}

		if (curIndexEntry->isFree && curIndexEntry->size > newSize
			&& (out_searchResult.targetEntry == nullptr || out_searchResult.targetEntry->size > curIndexEntry->size))
		{
			out_searchResult.targetEntry = curIndexEntry;
		}

		prevIndexEntry = curIndexEntry;
		curIndexEntry = curIndexEntry->nextEntry;
	}

	return foundDataPtr;
}


//---------------------------------------------------------------------------
bool BaseMemoryManager::FindIndexEntryFromDataPtr(void* dataPtr, IndexEntry*& out_indexEntry, IndexEntry*& out_prevIndexEntry)
{
	IndexEntry* curIndexEntry = m_memoryIndexHead;
	IndexEntry* prevIndexEntry = nullptr;
	while (curIndexEntry != nullptr)
	{
		if (curIndexEntry->dataLocation == dataPtr)
		{
			out_indexEntry = curIndexEntry;
			out_prevIndexEntry = prevIndexEntry;

			return true;
		}

		prevIndexEntry = curIndexEntry;
		curIndexEntry = curIndexEntry->nextEntry;
	}

	return false;
}


//---------------------------------------------------------------------------
void BaseMemoryManager::FreeAndCoalesceIndexEntry(IndexEntry* indexEntryToFree, IndexEntry* indexEntryToFreePrev)
{
	// Coalesce left
	if (indexEntryToFreePrev != nullptr
		&& indexEntryToFreePrev->isFree)
	{
		indexEntryToFreePrev->size = indexEntryToFreePrev->size + indexEntryToFree->size;
		indexEntryToFreePrev->nextEntry = indexEntryToFree->nextEntry;
		m_indexPool.Free(indexEntryToFree);
		indexEntryToFree = indexEntryToFreePrev;
	}

	// Coalesce right
	IndexEntry* nextIndexEntry = indexEntryToFree->nextEntry;
	if (nextIndexEntry != nullptr
		&& nextIndexEntry->isFree)
	{
		indexEntryToFree->size = indexEntryToFree->size + nextIndexEntry->size;
		indexEntryToFree->nextEntry = nextIndexEntry->nextEntry;
		m_indexPool.Free(nextIndexEntry);
	}

	// Reset freed entry
	indexEntryToFree->isFree = true;
	indexEntryToFree->dataLocation = indexEntryToFree->blockLocation;
}
