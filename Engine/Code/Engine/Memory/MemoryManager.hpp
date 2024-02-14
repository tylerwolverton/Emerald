#pragma once
#include "Engine/Memory/ObjectPool.hpp"
#include "Engine/Memory/LockPolicies.hpp"

#include <stdint.h>
#include <mutex>


//----------------------------------------------------------------------------
struct IndexEntry
{
public:
	IndexEntry(byte* ptrLocation, uint64_t size)
		: blockLocation(ptrLocation)
		, dataLocation(ptrLocation)
		, size(size)
	{}

	// Account for alignment by saving pointer to data as well as beginning of memory block
	IndexEntry(byte* blockPtrLocation, byte* dataPtrLocation, uint64_t size)
		: blockLocation(blockPtrLocation)
		, dataLocation(dataPtrLocation)
		, size(size)
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


//----------------------------------------------------------------------------
template<typename T>
class Handle;

// One page of cache
typedef ObjectPool<IndexEntry, 4096 / sizeof(IndexEntry)> IndexEntryObjectPool;


//----------------------------------------------------------------------------
class BaseMemoryManager
{
	template<typename T> friend class Handle;

public:
	template<typename T, typename ...Args>
	Handle<T> CreateHandle(Args&& ...args)
	{
		return Handle<T>(*this, std::forward<Args>(args)...);
	}

	virtual void* Reallocate(void* ptrToMemory, uint64_t newSize) = 0;
	virtual void  Free(void* ptrToMemory) = 0;
	virtual void  Defragment() = 0;

protected:
	virtual IndexEntry* AllocateNonMovable(int size, int count = 1, int alignment = 1) = 0;
	bool FindIndexEntryFromDataPtr(void* dataPtr, IndexEntry*& out_indexEntry, IndexEntry*& out_prevIndexEntry);
	bool FindIndexEntryFromDataPtrForRealloc(void* dataPtr, ReallocIndexEntrySearchResult& out_searchResult, uint64_t newSize);
	void FreeAndCoalesceIndexEntry(IndexEntry* indexEntryToFree, IndexEntry* indexEntryToFreePrev);

protected:
	byte* m_mainMemoryBlock = nullptr;
	uint64_t m_remainingMainMemory = 0;

	IndexEntryObjectPool m_indexPool;
	IndexEntry* m_memoryIndexHead = nullptr;

	std::mutex m_mutex;
};


//----------------------------------------------------------------------------
template<typename LockPolicy, typename MovementPolicy>
requires Defragmentable<MovementPolicy>
class MemoryManager : public BaseMemoryManager
{
public:
	MemoryManager() {}
	MemoryManager(byte* memBlockToManage, uint64_t memBlockSize);
	~MemoryManager();

	void Initialize(byte* memBlockToManage, uint64_t memBlockSize);

	virtual void* Allocate(int size, int count = 1, int alignment = 1);
	virtual void* Reallocate(void* ptrToMemory, uint64_t newSize) override;
	virtual void  Free(void* ptrToMemory) override;

	virtual void Defragment() override
	{
		LockPolicy lock(m_mutex);
		MovementPolicy::Defragment(m_memoryIndexHead, &m_indexPool);
	}

	void DebugPrint();

protected:
	virtual IndexEntry*	AllocateNonMovable(int size, int count = 1, int alignment = 1) override;
};


#include "MemoryManager.inl"