#include "Engine/Zephyr/GameInterface/ZephyrEngineEvents.hpp"
									

//-----------------------------------------------------------------------------------------------
ZephyrEngineEvents::~ZephyrEngineEvents()
{
	m_registeredMethods.clear();
}


//-----------------------------------------------------------------------------------------------
bool ZephyrEngineEvents::IsMethodRegistered( const std::string& methodName )
{
	auto iter = m_registeredMethods.find( methodName );
	
	return iter != m_registeredMethods.end();
}
