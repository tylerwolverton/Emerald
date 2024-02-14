#pragma once
#include <mutex>


//----------------------------------------------------------------------------
template <typename LockStorage, void (*LockFn)(LockStorage*), void (*UnlockFn)(LockStorage*)>
class LockPolicy
{
public:
	void Lock()
	{
		LockFn(&m_storage);
	}

	void Unlock()
	{
		UnlockFn(&m_storage);
	}

private:
	LockStorage m_storage;
};


//----------------------------------------------------------------------------
template <typename LockStorage, typename LockFnType, void (LockFnType::* LockFn)(), void (LockFnType::* UnlockFn)()>
class LockPolicyMember
{
public:
	void Lock()
	{
		(m_storage.*LockFn)();
	}

	void Unlock()
	{
		(m_storage.*UnlockFn)();
	}

private:
	LockStorage m_storage;
};


//----------------------------------------------------------------------------
template <typename LockStorage, typename LockFnType, void (LockFnType::* LockFn)(), void (LockFnType::* UnlockFn)()>
class ScopedLockPolicyMember
{
public:
	ScopedLockPolicyMember(LockStorage& lockStorage)
		: m_storage(lockStorage)
	{
		Lock();
	}
	
	~ScopedLockPolicyMember()
	{
		Unlock();
	}

	void Lock()
	{
		(m_storage.*LockFn)();
	}

	void Unlock()
	{
		(m_storage.*UnlockFn)();
	}

private:
	LockStorage& m_storage;
};


//----------------------------------------------------------------------------
typedef LockPolicyMember< std::mutex, std::_Mutex_base, &std::_Mutex_base::lock, &std::_Mutex_base::unlock> MutexPolicy;
typedef ScopedLockPolicyMember< std::mutex, std::_Mutex_base, &std::_Mutex_base::lock, &std::_Mutex_base::unlock> ScopedMutexPolicy;


//----------------------------------------------------------------------------
class NullLock
{
public:
	NullLock() {}
	NullLock(std::mutex& mutex) { (void)mutex; }
	~NullLock() {}
};
