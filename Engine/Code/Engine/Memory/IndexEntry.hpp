#pragma once
#include "Engine/Core/EngineCommon.hpp"


//----------------------------------------------------------------------------
struct IndexEntry
{
public:
	IndexEntry( byte* ptrLocation, uint64_t size )
		: blockLocation( ptrLocation )
		, dataLocation( ptrLocation )
		, size( size )
	{}

	// Account for alignment by saving pointer to data as well as beginning of memory block
	IndexEntry( byte* blockPtrLocation, byte* dataPtrLocation, uint64_t size )
		: blockLocation( blockPtrLocation )
		, dataLocation( dataPtrLocation )
		, size( size )
	{}

	byte* blockLocation = nullptr;
	byte* dataLocation = nullptr;
	IndexEntry* nextEntry = nullptr;
	uint64_t size = 0xDEADBEEF; // Includes padding
	int refCount = 1;
	bool isPinned = false;
	bool isFree = true;
};


//----------------------------------------------------------------------------
struct IndexEntrySearchResult
{
public:
	IndexEntry* entry = nullptr;
	IndexEntry* prevEntry = nullptr;
};


//----------------------------------------------------------------------------
struct ReallocIndexEntrySearchResult : public IndexEntrySearchResult
{
public:
	IndexEntry* targetEntry = nullptr;
};


// One page of cache
typedef ObjectPool<IndexEntry, 4096 / sizeof( IndexEntry )> IndexEntryObjectPool;
