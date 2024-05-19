#pragma once
#include <utility>


//----------------------------------------------------------------------------
struct IndexEntry;
class BaseMemoryManager;


//----------------------------------------------------------------------------
template<typename T>
class Handle
{
	template<typename T> friend class SmartPtr;
	template<typename T, typename ChildType> friend class ChildSmartPtr;

public:
	Handle() {}

	Handle( BaseMemoryManager& managerWhoCreatedMe )
		: m_managerWhoCreatedMe( &managerWhoCreatedMe )
	{}


	//----------------------------------------------------------------------------
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


	//----------------------------------------------------------------------------
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


	//----------------------------------------------------------------------------
	Handle( const Handle& other )
		: m_entry( other.m_entry )
	{
		if ( m_entry )
		{
			m_entry->refCount++;
			m_managerWhoCreatedMe = other.m_managerWhoCreatedMe;
		}
	}


	//----------------------------------------------------------------------------
	Handle( Handle&& other )
		: m_entry( other.m_entry )
	{
		if ( m_entry )
		{
			m_managerWhoCreatedMe = other.m_managerWhoCreatedMe;
		}
	}


	//----------------------------------------------------------------------------
	Handle& operator=( const Handle& other )
	{
		if ( m_entry )
		{
			Destroy();
		}

		if ( other.IsValid() )
		{
			other.m_entry->refCount++;
		}

		m_entry = other.m_entry;
		m_managerWhoCreatedMe = other.m_managerWhoCreatedMe;
		m_numPinned = other.m_numPinned;

		return *this;
	}


	//----------------------------------------------------------------------------
	Handle& operator=( Handle&& other )
	{
		if ( m_entry )
		{
			Destroy();
		}

		//other.m_entry->refCount++;

		m_entry = other.m_entry;
		m_managerWhoCreatedMe = other.m_managerWhoCreatedMe;
		m_numPinned = other.m_numPinned;

		other.m_entry = nullptr;

		return *this;
	}


	//----------------------------------------------------------------------------
	void MoveCopy( Handle& other )
	{
		if ( m_entry )
		{
			Destroy();
		}

		m_entry = other.m_entry;
		m_managerWhoCreatedMe = other.m_managerWhoCreatedMe;
		m_numPinned = other->m_numPinned;

		if ( m_entry )
		{
			other->Destroy();
		}
	}


	//----------------------------------------------------------------------------
	virtual ~Handle()
	{
		if ( IsValid() )
		{
			Destroy();
		}
	}


	//----------------------------------------------------------------------------
	bool IsValid() const
	{
		return m_entry != nullptr && m_entry->refCount > 0;
	}

protected:
	//----------------------------------------------------------------------------
	T* Pin()
	{
		if ( m_entry )
		{
			++m_numPinned;
			m_entry->isPinned = true;
			return (T*)m_entry->dataLocation;
		}

		return nullptr;
	}


	//----------------------------------------------------------------------------
	void Unpin()
	{
		if ( m_entry )
		{
			--m_numPinned;
			if ( m_numPinned <= 0 )
			{
				m_entry->isPinned = false;
			}
		}
	}


	//----------------------------------------------------------------------------
	void Destroy()
	{
		m_entry->refCount--;
		if ( m_entry->refCount == 0 )
		{
			if ( m_entry->isPinned )
			{
				m_entry->isPinned = false;
			}

			// Call destructor of allocated data
			( (T*)m_entry->dataLocation )->~T();
			m_managerWhoCreatedMe->Free( m_entry->dataLocation );
		}

		m_entry = nullptr;
	}

protected:
	IndexEntry* m_entry = nullptr;
	BaseMemoryManager* m_managerWhoCreatedMe = nullptr;
	int m_numPinned = 0;
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


	//----------------------------------------------------------------------------
	~SmartPtr()
	{
		m_handle.Unpin();
		m_pinnedData = nullptr;
	}

	// For now, don't allow copying smart pointers
	SmartPtr( const SmartPtr& other ) = delete;

	T* operator->() { return m_pinnedData; }


	//----------------------------------------------------------------------------
	bool IsNull() const
	{
		return m_pinnedData == nullptr;
	}

private:
	Handle<T>& m_handle;
	T* m_pinnedData = nullptr;
};


// This won't work with CRTP
////----------------------------------------------------------------------------
//// Support handles for child types
//template<class Child, class Parent>
//concept Derived = std::is_base_of<Parent, Child>::value;

//----------------------------------------------------------------------------
template<class ParentType, class ChildType>
class ChildSmartPtr
{
public:
	ChildSmartPtr( Handle<ParentType>& handle )
		: m_handle( handle )
		, m_pinnedData( (ChildType*)handle.Pin() ) {}


	//----------------------------------------------------------------------------
	~ChildSmartPtr()
	{
		m_handle.Unpin();
		m_pinnedData = nullptr;
	}

	// For now, don't allow copying smart pointers
	ChildSmartPtr( const ChildSmartPtr& other ) = delete;

	ChildType* operator->() { return m_pinnedData; }


	//----------------------------------------------------------------------------
	bool IsNull() const
	{
		return m_pinnedData == nullptr;
	}

private:
	Handle<ParentType>& m_handle;
	ChildType* m_pinnedData = nullptr;
};
