#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Clock;
struct ZephyrTypeMetadata;
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

	void		Startup( const ZephyrSystemParams& params );
	void		BeginFrame(){};
	void		Update();
	void		EndFrame() {};
	void		Shutdown();

	void		StartNewTimer( const EntityId& targetId, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs );
	void		StartNewTimer( const std::string& targetName, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs );
	void		StopAllTimers();

	void		RegisterZephyrType( const ZephyrTypeMetadata& typeMetadata );
	std::map<std::string, ZephyrTypeMetadata> GetRegisteredUserTypes()			{ return m_registeredZephyrTypes; }

private:
	void UpdateTimers();

private:
	Clock* m_clock = nullptr;
	// TODO: Use new ObjectPool with this once the engine has it
	std::vector<ZephyrTimer> m_timerPool;
	std::map<std::string, ZephyrTypeMetadata> m_registeredZephyrTypes;

};
