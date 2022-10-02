#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <unordered_set>
#include <string>


//-----------------------------------------------------------------------------------------------
class Entity;


//-----------------------------------------------------------------------------------------------
class ZephyrEngineEvents
{
public:
	ZephyrEngineEvents();
	virtual ~ZephyrEngineEvents();

	bool IsMethodRegistered( const std::string& methodName );

	virtual Entity* GetEntityById( const EntityId& id ) const = 0;
	virtual Entity* GetEntityByName( const std::string& name ) const = 0;

private:
	// Zephyr Script Events
	//void ChangeZephyrScriptState( EventArgs* args );

	//// Debug Events
	//void PrintDebugText( EventArgs* args );
	//void PrintDebugScreenText( EventArgs* args );
	//void PrintToConsole( EventArgs* args );

protected:
	std::unordered_set<std::string> m_registeredMethods;
};
