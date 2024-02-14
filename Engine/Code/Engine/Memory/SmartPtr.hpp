#pragma once
//----------------------------------------------------------------------------
struct IndexEntry;
class BaseMemoryManager;


//----------------------------------------------------------------------------
template<typename T>
class Handle
{
public:
	Handle(){}

	template<typename ...Args>
	Handle(BaseMemoryManager& managerWhoCreatedMe, Args&& ...args)
		: m_managerWhoCreatedMe(&managerWhoCreatedMe)
	{
		m_entry = managerWhoCreatedMe.AllocateNonMovable(sizeof(T), 1, alignof(T));
		if (m_entry == nullptr)
		{
			return;
		}

		m_entry->refCount = 1;
		m_entry->isPinned = false;
		new(m_entry->dataLocation) T(std::forward<Args>(args)...);
	}

	Handle(const Handle& other)
		: m_entry(other.m_entry)
	{
		m_entry->refCount++;
		m_managerWhoCreatedMe = other.m_managerWhoCreatedMe;
	}

	Handle& operator=(const Handle& other)
	{
		if (m_entry)
		{
			m_entry->refCount--;
		}

		other.m_entry->refCount++;

		m_entry = other.m_entry;
		m_managerWhoCreatedMe = other.m_managerWhoCreatedMe;

		return *this;
	}

	virtual ~Handle()
	{
		if (IsValid())
		{
			Destroy();
		}
	}

	T* Pin()
	{
		m_entry->isPinned = true;
		return (T*)m_entry->dataLocation;
	}

	void Unpin()
	{
		m_entry->isPinned = false;
	}

	// For testing defrag, review if this is a good thing to have
	void Destroy()
	{
		m_entry->refCount--;
		if (m_entry->refCount == 0)
		{
			// Call destructor of allocated data
			((T*)m_entry->dataLocation)->~T();
			m_managerWhoCreatedMe->Free(m_entry->dataLocation);
		}

		m_entry = nullptr;
	}

	bool IsValid() const
	{
		return m_entry != nullptr && m_entry->refCount > 0;
	}

protected:
	IndexEntry* m_entry = nullptr;
	BaseMemoryManager* m_managerWhoCreatedMe = nullptr;
};


//----------------------------------------------------------------------------
// Not like std smart pointers, really more of a HandleHelper class
template<typename T>
class SmartPtr
{
public:
	SmartPtr(Handle<T>& handle)
		: m_handle(handle)
		, m_pinnedData(handle.Pin()) {}

	~SmartPtr()
	{ 
		m_handle.Unpin();
	}

	// For now, don't allow copying smart pointers
	SmartPtr(const SmartPtr& other) = delete;

	T* operator->() { return m_pinnedData; }

private:
	Handle<T>& m_handle;
	T* m_pinnedData = nullptr;
};
