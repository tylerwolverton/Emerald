#include "Engine/Core/EngineCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Clock;
struct ZephyrTimer;


//-----------------------------------------------------------------------------------------------
struct ZephyrSystemParams
{
public:
	Clock* clock;
};


//-----------------------------------------------------------------------------------------------
class ZephyrSystem
{
public:
	ZephyrSystem();
	~ZephyrSystem();

	void		Startup( const ZephyrSystemParams& params );
	void		BeginFrame(){};
	void		Update();
	void		EndFrame() {};
	void		Shutdown();

	void		StartNewTimer( const EntityId& targetId, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs );
	void		StartNewTimer( const std::string& targetName, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs );
	void		StopAllTimers();

private:
	void UpdateTimers();

private:
	Clock* m_clock = nullptr;
	// TODO: Use new ObjectPool with this once the engine has it
	std::vector<ZephyrTimer> m_timerPool;
};
