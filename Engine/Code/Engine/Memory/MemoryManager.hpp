#pragma once
#include "Engine/Memory/ObjectPool.hpp"
#include "Engine/Memory/IndexEntry.hpp"
#include "Engine/Memory/LockPolicies.hpp"
#include "Engine/Memory/MovableMemory.hpp"

#include <stdint.h>
#include <mutex>


//----------------------------------------------------------------------------
template<typename T>
class Handle;


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


#include "Engine/Memory/MemoryManager.inl"