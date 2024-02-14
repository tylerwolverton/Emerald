#pragma once
typedef char byte;


// An object pool to manage the memory of a group of objects of the same type. 
// The caller still needs to initialize the objects themselves, this class simply 
// concerns itself with giving back raw memory in appropriately sized chunks.
template <typename T, int Size>
class ObjectPool
{
public:
	//----------------------------------------------------------------------------
	ObjectPool()
	{
		// Initialize objects array as another way to interpret the raw memory
		for (int i = 0; i < Size; ++i)
		{
			m_objects[i] = m_memoryStorage + ( sizeof(T) * i );
		}
	}

	//----------------------------------------------------------------------------
	~ObjectPool()
	{
		if (m_nextPool != nullptr)
		{
			delete m_nextPool;
		}
	}

private:
	// Don't allow any copies of a pool
	ObjectPool(const ObjectPool&) = delete;
	ObjectPool(ObjectPool&&) = delete;
	ObjectPool& operator=(const ObjectPool&) = delete;
	ObjectPool& operator=(ObjectPool&&) = delete;


	//----------------------------------------------------------------------------
	bool IsObjectInStorage(void* object)
	{
		byte* objAddr = (byte*)object;

		return objAddr >= m_memoryStorage
			&& objAddr < m_memoryStorage + Size;
	}

public:
	//----------------------------------------------------------------------------
	void* Allocate()
	{
		if (m_topOfStack >= 0)
		{
			return m_objects[m_topOfStack--];
		}

		if (m_nextPool == nullptr)
		{
			m_nextPool = new ObjectPool<T,Size>();
		}

		return m_nextPool->Allocate();
	}


	//----------------------------------------------------------------------------
	void Free(void* object)
	{		
		if (object == nullptr)
		{
			return;
		}

		// The object lives in this pool, place the requested free on top of the stack so it'll be reused by allocate
		// Note: the pointer will still point to the old memory until reallocated but the caller should consider it illegal to keep using it
		// since after freeing it's "invalid"
		if (IsObjectInStorage(object))
		{
			++m_topOfStack;
			m_objects[m_topOfStack] = object;
			return;
		}

		if (m_nextPool != nullptr)
		{
			m_nextPool->Free(object);
		}
	}

private:
	int m_topOfStack = Size - 1;
	byte m_memoryStorage[sizeof(T) * Size];
	void* m_objects[Size];

	ObjectPool* m_nextPool = nullptr;
};

