#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Memory/MemoryManager.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Clock;
class ZephyrTypeMetadata;
struct ZephyrTimer;


//-----------------------------------------------------------------------------------------------
struct ZephyrSystemParams
{
public:
	Clock* clock;
};


//-----------------------------------------------------------------------------------------------
class ZephyrSubsystem
{
public:
	ZephyrSubsystem() {}
	~ZephyrSubsystem() {}

	void										Startup( const ZephyrSystemParams& params );
	void										BeginFrame(){};
	void										Update();
	void										EndFrame() {};
	void										Shutdown();

	void										StartNewTimer( const EntityId& targetId, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs );
	void										StartNewTimer( const std::string& targetName, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs );
	void										StopAllTimers();

	void										RegisterZephyrType( ZephyrTypeMetadata* typeMetadata );
	ZephyrTypeMetadata*							GetRegisteredUserType( const std::string& typeName ) const;
	ZephyrTypeMetadata							GetCopyOfRegisteredUserType( const std::string& typeName ) const;
	void										ResetRegisteredUserTypes();

private:
	void UpdateTimers();

private:
	Clock* m_clock = nullptr;
	// TODO: Use new ObjectPool with this once the engine has it
	std::vector<ZephyrTimer> m_timerPool;

	// Needs to be string keyed map so the scanner and parser can keep track of types by name
	std::map<std::string, ZephyrTypeMetadata*> m_registeredZephyrTypes;

	byte* m_zephyrManagedMemoryBlock = nullptr;
	MemoryManager<NullLock, MovableMemory>* m_memoryMgr = nullptr;
};
