#include "Engine/Zephyr/GameInterface/ZephyrEngineEvents.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Framework/Entity.hpp"


//#define REGISTER_EVENT( eventName ) {\
//										m_registeredMethods.insert( #eventName );\
//										g_eventSystem->RegisterMethodEvent( #eventName, "", EVERYWHERE, this, &ZephyrEngineAPI::eventName );\
//									}

//-----------------------------------------------------------------------------------------------
ZephyrEngineEvents::ZephyrEngineEvents()
{
	//REGISTER_EVENT( ChangeZephyrScriptState );
	/*REGISTER_EVENT( PrintDebugText );
	REGISTER_EVENT( PrintDebugScreenText );
	REGISTER_EVENT( PrintToConsole );*/
}


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
