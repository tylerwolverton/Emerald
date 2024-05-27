#include "Engine/Zephyr/GameInterface/ZephyrEngineEvents.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"
#include "Engine/Zephyr/Types/ZephyrString.hpp"
#include "Engine/Zephyr/Types/ZephyrVec2.hpp"
									

#define REGISTER_EVENT( eventName ) {\
										m_registeredMethods.insert( #eventName );\
										g_eventSystem->RegisterMethodEvent( #eventName, "", SCRIPT, this, &ZephyrEngineEvents::eventName );\
									}


//-----------------------------------------------------------------------------------------------
ZephyrEngineEvents::ZephyrEngineEvents()
{
	REGISTER_EVENT( PrintDebugScreenText );
	REGISTER_EVENT( PrintToConsole );
	REGISTER_EVENT( Assert );
	REGISTER_EVENT( AssertEqual );
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


//-----------------------------------------------------------------------------------------------
/**
 * Print debug screen text at given position.
 *
 * params:
 *	- text: text to print
 *		- Zephyr type: String
 *	- duration: duration in seconds to display text
 *		- Zephyr type: Number
 *		- default: 0 ( single frame )
 *	- fontSize: height in pixels of font
 *		- Zephyr type: Number
 *		- default: 24
 *	- locationRatio: position of font on screen, x and y are between 0 and 1
 *		- Zephyr type: Vec2
 *		- default: ( 0, 0 )
 *	- padding: how much padding in pixels to add to text position
 *		- Zephyr type: Vec2
 *		- default: ( 0, 0 )
 *	- color: name of color to print in ( supported colors: white, red, green, blue, black )
 *		- Zephyr type: String
 *		- default: "white"
*/
//-----------------------------------------------------------------------------------------------
void ZephyrEngineEvents::PrintDebugScreenText( EventArgs* args )
{
	std::string text = args->GetValue( "text", "" );
	float duration = args->GetValue( "duration", 0.f );
	float fontSize = args->GetValue( "fontSize", 24.f );
	Vec2 locationRatio = args->GetValue( "locationRatio", Vec2::ZERO );
	Vec2 padding = args->GetValue( "padding", Vec2::ZERO );

	std::string colorStr = args->GetValue( "color", "white" );

	Rgba8 color = Rgba8::WHITE;
	if ( colorStr == "white" ) { color = Rgba8::WHITE; }
	else if ( colorStr == "red" ) { color = Rgba8::RED; }
	else if ( colorStr == "green" ) { color = Rgba8::GREEN; }
	else if ( colorStr == "blue" ) { color = Rgba8::BLUE; }
	else if ( colorStr == "black" ) { color = Rgba8::BLACK; }

	DebugAddScreenText( Vec4( locationRatio, padding ), Vec2::ZERO, fontSize, color, color, duration, text.c_str() );
}


//-----------------------------------------------------------------------------------------------
/**
 * Print text to dev console.
 *
 * params:
 *	- text: text to print
 *		- Zephyr type: String
 *	- color: name of color to print in ( supported colors: white, red, green, blue, black )
 *		- Zephyr type: String
 *		- default: "white"
*/
//-----------------------------------------------------------------------------------------------
void ZephyrEngineEvents::PrintToConsole( EventArgs* args )
{
	ZephyrValue textType = args->GetValue( "text", ZephyrValue::NULL_VAL );
	std::string text;
	textType.TryToGetValueFrom<ZephyrString>( text );

	ZephyrValue colorHandle = args->GetValue( "color", ZephyrValue::NULL_VAL );
	std::string colorStr = "white";
	colorHandle.TryToGetValueFrom<ZephyrString>( colorStr );

	Rgba8 color = Rgba8::WHITE;
	if ( colorStr == "white" ) { color = Rgba8::WHITE; }
	else if ( colorStr == "red" ) { color = Rgba8::RED; }
	else if ( colorStr == "green" ) { color = Rgba8::GREEN; }
	else if ( colorStr == "blue" ) { color = Rgba8::BLUE; }
	else if ( colorStr == "black" ) { color = Rgba8::BLACK; }

	g_devConsole->PrintString( text.c_str(), color );
}


//-----------------------------------------------------------------------------------------------
void ZephyrEngineEvents::Assert( EventArgs* args )
{
	ZephyrValue errorMsgType = args->GetValue( "errorMsg", ZephyrValue::NULL_VAL );
	std::string errorMsg;
	errorMsgType.TryToGetValueFrom<ZephyrString>( errorMsg );

	ZephyrValue resultType = args->GetValue( "result", ZephyrValue::NULL_VAL );
	
	if ( !resultType.EvaluateAsBool() )
	{
		g_devConsole->PrintError( errorMsg );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrEngineEvents::AssertEqual( EventArgs* args )
{
	ZephyrValue errorMsgType = args->GetValue( "errorMsg", ZephyrValue::NULL_VAL );
	std::string errorMsg;
	errorMsgType.TryToGetValueFrom<ZephyrString>( errorMsg );

	ZephyrValue aType = args->GetValue( "a", ZephyrValue::NULL_VAL );
	ZephyrValue bType = args->GetValue( "b", ZephyrValue::NULL_VAL );
	
	if ( !aType.IsValid() || !bType.IsValid() )
	{
		return;
	}
		
	ZephyrValue result = aType.Equal( bType );
	if ( !result.EvaluateAsBool() )
	{
		g_devConsole->PrintError( Stringf( "Assert failed %s == %s, %s", aType.ToString().c_str(), bType.ToString().c_str(), errorMsg.c_str() ));
	}
}

