#pragma once
//----------------------------------------------------------------------------
struct IndexEntry;
class BaseMemoryManager;


//----------------------------------------------------------------------------
template<typename T>
class Handle
{
public:
	Handle( BaseMemoryManager& managerWhoCreatedMe )
		: m_managerWhoCreatedMe( &managerWhoCreatedMe )
	{}

	template<typename ...Args>
	void Initialize( Args&& ...args )
	{
		m_entry = m_managerWhoCreatedMe->AllocateNonMovable( sizeof( T ), 1, alignof( T ) );
		if ( m_entry == nullptr )
		{
			return;
		}

		m_entry->refCount = 1;
		m_entry->isPinned = false;
		new( m_entry->dataLocation ) T( std::forward<Args>( args )... );
	}

	template<typename ChildType, typename ...Args>
	void InitializeAsParent( Args&& ...args )
	{
		m_entry = m_managerWhoCreatedMe->AllocateNonMovable( sizeof( ChildType ), 1, alignof( ChildType ) );
		if ( m_entry == nullptr )
		{
			return;
		}

		m_entry->refCount = 1;
		m_entry->isPinned = false;
		new( m_entry->dataLocation ) ChildType( std::forward<Args>( args )... );
	}

	Handle( const Handle& other )
		: m_entry( other.m_entry )
	{
		m_entry->refCount++;
		m_managerWhoCreatedMe = other.m_managerWhoCreatedMe;
	}

	Handle& operator=( const Handle& other )
	{
		if ( m_entry )
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
		if ( IsValid() )
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
		if ( m_entry->refCount == 0 )
		{
			// Call destructor of allocated data
			( (T*)m_entry->dataLocation )->~T();
			m_managerWhoCreatedMe->Free( m_entry->dataLocation );
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
	SmartPtr( Handle<T>& handle )
		: m_handle( handle )
		, m_pinnedData( handle.Pin() ) {}

	~SmartPtr()
	{
		m_handle.Unpin();
	}

	// For now, don't allow copying smart pointers
	SmartPtr( const SmartPtr& other ) = delete;

	T* operator->() { return m_pinnedData; }

private:
	Handle<T>& m_handle;
	T* m_pinnedData = nullptr;
};


//----------------------------------------------------------------------------
// Support handles for child types
template<typename ParentType, typename ChildType>
class ChildSmartPtr
{
public:
	ChildSmartPtr( Handle<ParentType>& handle )
		: m_handle( handle )
		, m_pinnedData( (ChildType*)handle.Pin() ) {}

	~ChildSmartPtr()
	{
		m_handle.Unpin();
		m_pinnedData = nullptr;
	}

	// For now, don't allow copying smart pointers
	ChildSmartPtr( const ChildSmartPtr& other ) = delete;

	ChildType* operator->() { return m_pinnedData; }

private:
	Handle<ParentType>& m_handle;
	ChildType* m_pinnedData = nullptr;
};

//#pragma once
//#include "Engine/Memory/MemoryManager.hpp"
//
//
////----------------------------------------------------------------------------
//class HandleBase
//{
//public:
//	HandleBase( BaseMemoryManager* managerWhoCreatedMe )
//		: m_managerWhoCreatedMe( managerWhoCreatedMe ) 
//	{}
//	HandleBase( const HandleBase& other );
//	virtual ~HandleBase();
//
//	HandleBase& operator=( const HandleBase& other );
//
//	// For testing defrag, review if this is a good thing to have
//	void Destroy();
//	void Unpin();
//	bool IsValid() const;
//
//	virtual void CallDestructor() = 0;
//
//protected:
//	IndexEntry* m_entry = nullptr;
//	BaseMemoryManager* m_managerWhoCreatedMe = nullptr;
//};
//
//
////----------------------------------------------------------------------------
//template<typename T>
//class Handle : public HandleBase
//{
//public:
//	template<typename ...Args>
//	Handle(BaseMemoryManager* managerWhoCreatedMe, Args&& ...args)
//		: HandleBase( *managerWhoCreatedMe )
//	{
//		m_entry = managerWhoCreatedMe->AllocateNonMovable(sizeof(T), 1, alignof(T));
//		if (m_entry == nullptr)
//		{
//			return;
//		}
//
//		m_entry->refCount = 1;
//		m_entry->isPinned = false;
//		new(m_entry->dataLocation) T(std::forward<Args>(args)...);
//	}
//
//	//template<typename ChildType, typename ...Args>
//	//Handle(BaseMemoryManager& managerWhoCreatedMe, Args&& ...args)
//	//	: m_managerWhoCreatedMe(&managerWhoCreatedMe)
//	//{
//	//	m_entry = managerWhoCreatedMe.AllocateNonMovable(sizeof(ChildType), 1, alignof(ChildType));
//	//	if (m_entry == nullptr)
//	//	{
//	//		return;
//	//	}
//
//	//	m_entry->refCount = 1;
//	//	m_entry->isPinned = false;
//	//	new(m_entry->dataLocation) ChildType(std::forward<Args>(args)...);
//	//}
//
//	virtual void CallDestructor() override
//	{
//		( (T*)m_entry->dataLocation )->~T();
//	}
//
//	T* Pin()
//	{
//		m_entry->isPinned = true;
//		return (T*)m_entry->dataLocation;
//	}
//};
//
//
////----------------------------------------------------------------------------
//template<typename ParentType>
//class ParentHandle : public HandleBase
//{
//public:
//	ParentHandle( BaseMemoryManager* managerWhoCreatedMe )
//		: HandleBase( managerWhoCreatedMe )
//	{}
//
//	template<typename ParentType, typename ChildType, typename ...Args>
//	void Initialize( Args&& ...args )
//	{
//		m_entry = m_managerWhoCreatedMe->AllocateNonMovable( sizeof( ChildType ), 1, alignof( ChildType ) );
//		if ( m_entry == nullptr )
//		{
//			return;
//		}
//
//		m_entry->refCount = 1;
//		m_entry->isPinned = false;
//		new( m_entry->dataLocation ) ChildType( std::forward<Args>( args )... );
//	}
//
//	virtual void CallDestructor() override
//	{
//		( (ParentType*)m_entry->dataLocation )->~ParentType();
//	}
//
//	ParentType* Pin()
//	{
//		m_entry->isPinned = true;
//		return (ParentType*)m_entry->dataLocation;
//	}
//};
//
//
////----------------------------------------------------------------------------
//// Not like std smart pointers, really more of a HandleHelper class
//template<typename T>
//class SmartPtr
//{
//public:
//	SmartPtr(Handle<T>& handle)
//		: m_handle(handle)
//		, m_pinnedData(handle.Pin()) {}
//
//	~SmartPtr()
//	{ 
//		m_handle.Unpin();
//		m_pinnedData = nullptr;
//	}
//
//	// For now, don't allow copying smart pointers
//	SmartPtr(const SmartPtr& other) = delete;
//
//	T* operator->() { return m_pinnedData; }
//
//private:
//	Handle<T>& m_handle;
//	T* m_pinnedData = nullptr;
//};
//
//
////----------------------------------------------------------------------------
//// Support handles for child types
//template<typename ChildType>
//class ChildSmartPtr
//{
//public:
//	template<typename ParentType>
//	ChildSmartPtr( ParentHandle<ParentType>& handle )
//		: m_handle( (HandleBase*)(&handle) )
//		, m_pinnedData( (ChildType*)handle.Pin() ) {}
//
//	~ChildSmartPtr()
//	{
//		m_handle->Unpin();
//		m_pinnedData = nullptr;
//	}
//
//	// For now, don't allow copying smart pointers
//	ChildSmartPtr( const ChildSmartPtr& other ) = delete;
//
//	ChildType* operator->() { return m_pinnedData; }
//
//private:
//	HandleBase* m_handle = nullptr;
//	ChildType* m_pinnedData = nullptr;
//};
