#include "Engine/Zephyr/GameInterface/ZephyrEventSystem.hpp"


//-----------------------------------------------------------------------------------------------
void ZephyrEventSystem::RegisterGlobalEvent( const std::string& name, EventCallbackFunctionPtrType callback )
{
	HashedString hashStr( name );
	for ( ZephyrEvent& zephyrEvent : m_globalEvents )
	{
		if ( hashStr == zephyrEvent.name )
		{
			zephyrEvent.callbacks.push_back( callback );
			return;
		}
	}

	// Add a new event
	ZephyrEvent newEvent;
	newEvent.name = hashStr;
	newEvent.callbacks.push_back( callback );
}


