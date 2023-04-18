#include "Game/Scripting/ZephyrGameEvents.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"

#include "Game/Core/GameCommon.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/Map.hpp"
#include "Game/Framework/GameEntity.hpp"
#include "Game/Graphics/SpriteAnimationSetDefinition.hpp"


#define REGISTER_EVENT( eventName ) {\
										m_registeredMethods.insert( #eventName );\
										g_eventSystem->RegisterMethodEvent( #eventName, "", EVERYWHERE, this, &ZephyrGameEvents::eventName );\
									}

//-----------------------------------------------------------------------------------------------
ZephyrGameEvents::ZephyrGameEvents()
{
	//REGISTER_EVENT( ChangeZephyrScriptState );
	REGISTER_EVENT( PrintDebugText );
	REGISTER_EVENT( PrintDebugScreenText );
	REGISTER_EVENT( PrintToConsole );

	REGISTER_EVENT( GetVec2Normalized );

	REGISTER_EVENT( GetNativeEntityVariable );
	REGISTER_EVENT( SetNativeEntityVariable );
	REGISTER_EVENT( GetEntityUnderMouse );
	REGISTER_EVENT( DestroySelf );
	REGISTER_EVENT( StartDialogue );
	REGISTER_EVENT( EndDialogue );
	REGISTER_EVENT( StartNewTimer );
	REGISTER_EVENT( WinGame );

	REGISTER_EVENT( MoveToLocation );
	REGISTER_EVENT( PathToLocation );
	REGISTER_EVENT( MoveInDirection );
	//REGISTER_EVENT( GetEntityLocation );

	REGISTER_EVENT( SpawnEntity );

	REGISTER_EVENT( RegisterKeyEvent );
	REGISTER_EVENT( UnRegisterKeyEvent );
	REGISTER_EVENT( GetMouseCursorPositionWorld );
	REGISTER_EVENT( GetMouseCursorPositionUI );

	REGISTER_EVENT( ChangeSpriteAnimation );
	REGISTER_EVENT( PlaySpriteAnimation );
	REGISTER_EVENT( PlaySound );
	REGISTER_EVENT( ChangeMusic );
	REGISTER_EVENT( AddScreenShake );

	REGISTER_EVENT( AddAnimationEvent );
}


//-----------------------------------------------------------------------------------------------
ZephyrGameEvents::~ZephyrGameEvents()
{
}


//-----------------------------------------------------------------------------------------------
Entity* ZephyrGameEvents::GetEntityById( const EntityId& id ) const
{
	return g_game->GetEntityById( id );
}


//-----------------------------------------------------------------------------------------------
Entity* ZephyrGameEvents::GetEntityByName( const std::string& name ) const
{
	return g_game->GetEntityByName( name );
}


//-----------------------------------------------------------------------------------------------
/**
 * Destroys the entity who called this event.
 */
//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::DestroySelf( EventArgs* args )
{
	GameEntity* entity = GetTargetEntityFromArgs( args );

	if ( entity != nullptr )
	{
		entity->Die();
	}
}


//-----------------------------------------------------------------------------------------------
/**
 * Change the game state to Dialogue to initiate a dialogue with an NPC.
 */
//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::StartDialogue( EventArgs* args )
{
	UNUSED( args );

	g_game->ChangeGameState( eGameState::DIALOGUE );
}


//-----------------------------------------------------------------------------------------------
/**
 * Change the game state back to Playing to end a dialogue sequence.
 */
//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::EndDialogue( EventArgs* args )
{
	UNUSED( args );

	g_game->ChangeGameState( eGameState::PLAYING );
}


//-----------------------------------------------------------------------------------------------
/**
 * Start a new timer to fire an event on completion.
 *
 * params:
 * Target will be determined by the following optional parameters, checking in order the targetId, then targetName, then ( if neither name or id are specified ) targeting the entity who called this event.
 *	- targetId: id of target entity
 *		- Zephyr type: Number
 *	- targetName: name of target entity
 *		- Zephyr type: String
 *
 *	- name: the name of the timer ( currently unused, reserved for future use  )
 *		- Zephyr type: String
 *	- durationSeconds: length of time in seconds of timer
 *		- Zephyr type: Number
 *	- onCompletedEvent: the name of the event to fire upon timer completion
 *		- Zephyr type: String
 *	- broadcastEventToAll: when true, broadcast the onCompletedEvent to all entities ( this will override any target entity set )
 *		- Zephyr type: Bool
 *		- default: false
*/
//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::StartNewTimer( EventArgs* args )
{
	std::string timerName = args->GetValue( "name", "" );
	float durationSeconds = args->GetValue( "durationSeconds", 1.f );
	std::string onCompletedEventName = args->GetValue( "onCompletedEvent", "" );
	bool broadcastEventToAll = args->GetValue( "broadcastEventToAll", false );
	
	// Broadcast event to all takes precedence and broadcasts to all entities
	if ( broadcastEventToAll )
	{
		g_zephyrSubsystem->StartNewTimer( -1, timerName, durationSeconds, onCompletedEventName, args );
		return;
	}
	
	GameEntity* entity = GetTargetEntityFromArgs( args );
	if ( entity == nullptr )
	{
		return;
	}

	g_zephyrSubsystem->StartNewTimer( entity->GetId(), timerName, durationSeconds, onCompletedEventName, args );
}


////-----------------------------------------------------------------------------------------------
///**
// * Change the current State of the Zephyr script for the entity who called the event.
// *	Note: Called like ChangeState( newState ) in script as a built in function
// *
// * params:
// *	- targetState: the name of the Zephyr State to change to
// *		- Zephyr type: String
//*/
////-----------------------------------------------------------------------------------------------
//void ZephyrGameEvents::ChangeZephyrScriptState( EventArgs* args )
//{
//	std::string targetState = args->GetValue( "targetState", "" );
//	GameEntity* entity = GetTargetEntityFromArgs( args );
//
//	if ( entity != nullptr
//		 && !targetState.empty() )
//	{
//		entity->ChangeZephyrScriptState( targetState );
//	}
//}


//-----------------------------------------------------------------------------------------------
/**
 * Print debug world text at position of entity who called the event.
 *
 * params:
 *	- text: text to print
 *		- Zephyr type: String
 *	- duration: duration in seconds to display text
 *		- Zephyr type: Number
 *		- default: 0 ( single frame )
 *	- color: name of color to print in ( supported colors: white, red, green, blue, black )
 *		- Zephyr type: String
 *		- default: "white"
*/
//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::PrintDebugText( EventArgs* args )
{
	std::string text = args->GetValue( "text", "TestPrint" );
	float duration = args->GetValue( "duration", 0.f );
	GameEntity* entity = GetTargetEntityFromArgs( args );

	std::string colorStr = args->GetValue( "color", "white" );

	Rgba8 color = Rgba8::WHITE;
	if ( colorStr == "white" )		{ color = Rgba8::WHITE; }
	else if ( colorStr == "red" )	{ color = Rgba8::RED; }
	else if ( colorStr == "green" ) { color = Rgba8::GREEN; }
	else if ( colorStr == "blue" )  { color = Rgba8::BLUE; }
	else if ( colorStr == "black" ) { color = Rgba8::BLACK; }

	Mat44 textLocation;

	if ( entity != nullptr )
	{
		textLocation.SetTranslation2D( entity->GetPosition().XY() );
	}
	
	DebugAddWorldText( textLocation, Vec2::HALF, color, color, duration, .1f, eDebugRenderMode::DEBUG_RENDER_ALWAYS, text.c_str() );
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
void ZephyrGameEvents::PrintDebugScreenText( EventArgs* args )
{
	std::string text = args->GetValue( "text", "" );
	float duration = args->GetValue( "duration", 0.f );
	float fontSize = args->GetValue( "fontSize", 24.f );
	Vec2 locationRatio = args->GetValue( "locationRatio", Vec2::ZERO );
	Vec2 padding = args->GetValue( "padding", Vec2::ZERO );

	std::string colorStr = args->GetValue( "color", "white" );

	Rgba8 color = Rgba8::WHITE;
	if ( colorStr == "white" )		{ color = Rgba8::WHITE; }
	else if ( colorStr == "red" )	{ color = Rgba8::RED; }
	else if ( colorStr == "green" ) { color = Rgba8::GREEN; }
	else if ( colorStr == "blue" )	{ color = Rgba8::BLUE; }
	else if ( colorStr == "black" )	{ color = Rgba8::BLACK; }
	
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
void ZephyrGameEvents::PrintToConsole( EventArgs* args )
{
	std::string text = args->GetValue( "text", "" );
	std::string colorStr = args->GetValue( "color", "white" );

	Rgba8 color = Rgba8::WHITE;
	if		( colorStr == "white" ) { color = Rgba8::WHITE; }
	else if ( colorStr == "red" )	{ color = Rgba8::RED; }
	else if ( colorStr == "green" )	{ color = Rgba8::GREEN; }
	else if ( colorStr == "blue" )	{ color = Rgba8::BLUE; }
	else if ( colorStr == "black" ) { color = Rgba8::BLACK; }
	
	g_devConsole->PrintString( text.c_str(), color );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::GetVec2Normalized( EventArgs* args )
{
	Vec2 inVec = args->GetValue( "inVec", Vec2::ZERO );

	float length = inVec.GetLength();
	Vec2 direction = inVec.GetNormalized();

	args->SetValue( "length", length );
	args->SetValue( "direction", direction );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::SpawnEntity( EventArgs* args )
{
	GameEntity* entity = GetTargetEntityFromArgs( args );
	if ( entity == nullptr )
	{
		return;
	}

	std::string entityType = args->GetValue( "type", "" );
	if ( entityType.empty() )
	{
		return;
	}

	std::string name = args->GetValue( "name", "" );
	std::string mapName = args->GetValue( "map", "" );
	Vec2 position = args->GetValue( "position", entity->GetPosition().XY() );

	Map* mapToSpawnIn = entity->GetMap();
	if ( mapToSpawnIn == nullptr )
	{
		mapToSpawnIn = g_game->GetCurrentMap();
	}

	if ( !mapName.empty() )
	{
		mapToSpawnIn = g_game->GetMapByName( mapName );
		if ( mapToSpawnIn == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Can't spawn entity in nonexistent map '%s'", mapName.c_str() ) );
			return;
		}
	}

	GameEntity* newEntity = mapToSpawnIn->SpawnNewEntityFromNameAtPosition( entityType, position );
	/*newEntity->SetOrientationDegrees( orientation );
	newEntity->SetName( name );

	g_game->SaveEntityByName( newEntity );
	
	newEntity->InitializeZephyrEntityVariables();*/
	newEntity->FireSpawnEvent();

	if ( mapToSpawnIn == g_game->GetCurrentMap() )
	{
		newEntity->Load();
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::WinGame( EventArgs* args )
{
	UNUSED( args );

	g_game->ChangeGameState( eGameState::VICTORY );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::GetNativeEntityVariable( EventArgs* args )
{
	GameEntity* entity = GetTargetEntityFromArgs( args );
	std::string varName = args->GetValue( "varName", "" );
	ZephyrValue zephyrValue = args->GetValue( "zephyrValue", ZephyrValue() );

	if ( entity == nullptr || varName.empty() )
	{
		return;
	}

	bool isNative = false;
	if ( varName == "position" ) { zephyrValue = ZephyrValue( entity->GetPosition() ); isNative = true; }

	args->SetValue( "zephyrValue", zephyrValue );
	args->SetValue( "isNative", isNative );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::SetNativeEntityVariable( EventArgs* args )
{
	GameEntity* entity = GetTargetEntityFromArgs( args );
	std::string varName = args->GetValue( "varName", "" );
	ZephyrValue zephyrValue = args->GetValue( "zephyrValue", ZephyrValue() );

	if ( entity == nullptr || varName.empty() )
	{
		return;
	}

	bool isNative = false;
	if ( varName == "position" ) { entity->SetPosition( zephyrValue.GetAsVec3() ); isNative = true; }

	args->SetValue( "isNative", isNative );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::GetEntityUnderMouse( EventArgs* args )
{
	Vec2 mouseWorldPosition = g_game->GetMouseWorldPosition();

	GameEntity* entity = g_game->GetEntityAtPosition( mouseWorldPosition );
	
	EntityId foundEntityId = -1;
	if ( entity != nullptr )
	{
		foundEntityId = entity->GetId();
	}

	args->SetValue( "foundEntity", foundEntityId );
}


//-----------------------------------------------------------------------------------------------
/**
 * Move an entity towards a location.
 *
 * params:
 *	- pos: target position
 *		- Zephyr type: Number
 *	- speed: speed to move the entity
 *		- Zephyr type: Number
 *		- default: entity's default movement speed
*/
//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::MoveToLocation( EventArgs* args )
{
	GameEntity* entity = GetTargetEntityFromArgs( args );

	if ( entity == nullptr )
	{
		return;
	}

	Vec3 targetPos = args->GetValue( "pos", Vec3::ZERO );

	Vec3 moveDirection = targetPos - entity->GetPosition();
	moveDirection.Normalize();

	entity->SetPosition( targetPos );
	//float speed = args->GetValue( "speed", entity->GetSpeed() );

	//entity->MoveWithPhysics( speed, moveDirection );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::PathToLocation( EventArgs* args )
{
	GameEntity* entity = GetTargetEntityFromArgs( args );

	if ( entity == nullptr )
	{
		return;
	}

	Vec3 targetPos = args->GetValue( "pos", Vec3::ZERO );

	Vec3 moveDirection = targetPos - entity->GetPosition();
	moveDirection.Normalize();

	// TODO: Path around environment to location
}


//-----------------------------------------------------------------------------------------------
/**
 * Move an entity in a direction.
 *
 * params:
 *	- direction: target direction
 *		- Zephyr type: Vec2
 *	- speed: speed to move the entity
 *		- Zephyr type: Number
 *		- default: entity's default movement speed
*/
//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::MoveInDirection( EventArgs* args )
{
	GameEntity* entity = GetTargetEntityFromArgs( args );
	if ( entity == nullptr )
	{
		return;
	}

	Vec2 direction = args->GetValue( "direction", Vec2::ZERO );
	if ( direction == Vec2::ZERO )
	{
		return;
	}

	direction.Normalize();

	//float speed = args->GetValue( "speed", entity->GetSpeed() );

	//entity->MoveWithPhysics( speed, direction );
}


////-----------------------------------------------------------------------------------------------
///**
// * Gets the position of the target entity. 
// * 
// * returns:
// *	- Fires the EntityLocationUpdated event with a parameter "entityPos" containing a Vec2 with the requested position
// *
// * params:
// *	Target will be determined by the following optional parameters, checking in order the targetId, then targetName, then ( if neither name or id are specified ) targeting the entity who called this event.
// *	- targetId: id of target entity
// *		- Zephyr type: Number
// *	- targetName: name of target entity
// *		- Zephyr type: String
//*/
////-----------------------------------------------------------------------------------------------
//void ZephyrGameEvents::GetEntityLocation( EventArgs* args )
//{
//	EntityId targetId = args->GetValue( "target", (EntityId)-1 );
//	GameEntity* targetEntity = g_game->GetEntityById( targetId );
//  GameEntity* entity = GetTargetEntityFromArgs( args );
//
//	if ( entity == nullptr
//		 || targetEntity == nullptr )
//	{
//		return;
//	}
//
//	EventArgs targetArgs;
//	targetArgs.SetValue( "entityPos", targetEntity->GetPosition() );
//
//	entity->FireScriptEvent( "EntityLocationUpdated", &targetArgs );
//}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::RegisterKeyEvent( EventArgs* args )
{
	GameEntity* entity = GetTargetEntityFromArgs( args );
	if ( entity == nullptr )
	{
		return;
	}

	std::string key = args->GetValue( "key", "" );
	if ( key.empty() )
	{
		return;
	}

	std::string event = args->GetValue( "event", "" );
	if ( event.empty() )
	{
		return;
	}

	entity->RegisterKeyEvent( key, event );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::UnRegisterKeyEvent( EventArgs* args )
{
	GameEntity* entity = GetTargetEntityFromArgs( args );
	if ( entity == nullptr )
	{
		return;
	}

	std::string key = args->GetValue( "key", "" );
	if ( key.empty() )
	{
		return;
	}

	std::string event = args->GetValue( "event", "" );
	if ( event.empty() )
	{
		return;
	}

	entity->UnRegisterKeyEvent( key, event );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::GetMouseCursorPositionWorld( EventArgs* args )
{
	Vec2 mouseWorldPosition = g_game->GetMouseWorldPosition();

	args->SetValue( "pos", mouseWorldPosition );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::GetMouseCursorPositionUI( EventArgs* args )
{
	Vec2 mouseUIPosition = g_game->GetMouseUIPosition();

	args->SetValue( "pos", mouseUIPosition );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::ChangeSpriteAnimation( EventArgs* args )
{
	std::string newAnim = args->GetValue( "newAnim", "" );
	//Entity* targetEntity = g_game->GetEntityByName( targetId );
	GameEntity* entity = GetTargetEntityFromArgs( args );

	if ( entity == nullptr
		 || newAnim.empty() )
	{
		return;
	}

	entity->ChangeSpriteAnimation( newAnim );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::PlaySpriteAnimation( EventArgs* args )
{
	std::string newAnim = args->GetValue( "newAnim", "" );
	//Entity* targetEntity = g_game->GetEntityByName( targetId );
	GameEntity* entity = GetTargetEntityFromArgs( args );

	if ( entity == nullptr
		 || newAnim.empty() )
	{
		return;
	}

	entity->PlaySpriteAnimation( newAnim );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::PlaySound( EventArgs* args )
{
	std::string soundName = args->GetValue( "name", "" );
	if ( soundName.empty() )
	{
		g_devConsole->PrintError( Stringf( "PlaySound must specify \"name\" parameter" ) );
		return;
	}

	float volume = args->GetValue( "volume", 1.f );
	float balance = args->GetValue( "balance", 0.f );
	float speed = args->GetValue( "speed", 1.f );

	g_game->PlaySoundByName( soundName, false, volume, balance, speed );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::ChangeMusic( EventArgs* args )
{
	std::string musicName = args->GetValue( "musicName", "" );
	if ( musicName.empty() )
	{
		return;
	}

	float volume = args->GetValue( "volume", 1.f );
	float balance = args->GetValue( "balance", 0.f );
	float speed = args->GetValue( "speed", 1.f );

	g_game->ChangeMusic( musicName, true, volume, balance, speed );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::AddScreenShake( EventArgs* args )
{
	float intensity = args->GetValue( "intensity", 0.f );

	g_game->AddScreenShakeIntensity( intensity );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::AddAnimationEvent( EventArgs* args )
{
	GameEntity* entity = GetTargetEntityFromArgs( args );
	std::string animName = args->GetValue( "anim", "" );
	std::string frameStr = args->GetValue( "frame", "first" );
	std::string eventName = args->GetValue( "event", "" );

	if ( entity == nullptr
		 || animName.empty()
		 || eventName.empty() )
	{
		// print warning
		return;
	}

	//SpriteAnimationSetDefinition* spriteAnimSetDef = entity->GetSpriteAnimSetDef( animName );
	//if ( spriteAnimSetDef == nullptr )
	//{
	//	// Print error
	//	return;
	//}

	//int numFrames = spriteAnimSetDef->GetNumFrames();
	//if ( numFrames == 0 )
	//{
	//	// print error
	//	return;
	//}

	//int frameNum = 0;
	//if ( frameStr == "first" )
	//{
	//	frameNum = 0;
	//}
	//else if ( frameStr == "last" )
	//{
	//	frameNum = numFrames - 1;
	//}
	//else
	//{
	//	frameNum = FromString( frameStr, frameNum );
	//}

	//if ( frameNum >= numFrames )
	//{
	//	// print warning
	//	frameNum = numFrames - 1;
	//}

	//spriteAnimSetDef->AddFrameEvent( frameNum, eventName );
}


//-----------------------------------------------------------------------------------------------
GameEntity* ZephyrGameEvents::GetTargetEntityFromArgs( EventArgs* args )
{
	EntityId targetId = (EntityId)args->GetValue( TARGET_ENTITY_STR, INVALID_ENTITY_ID );
	std::string targetName = args->GetValue( TARGET_ENTITY_NAME_STR, "" );
	EntityId entityId = args->GetValue( PARENT_ENTITY_ID_STR, INVALID_ENTITY_ID );
	GameEntity* entity = (GameEntity*)g_game->GetEntityById( entityId );
	
	// Named entities are returned first
	if ( !targetName.empty() )
	{
		entity = g_game->GetEntityByName( targetName );
		if ( entity == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Failed to find an entity with name '%s'", targetName.c_str() ) );
			return nullptr;
		}
	}
	// Id entities are tried next
	else if ( targetId != -1 )
	{
		entity = g_game->GetEntityById( targetId );
		if ( entity == nullptr )
		{
			g_devConsole->PrintWarning( Stringf( "Failed to find an entity with id '%i'", targetId ) );
			return nullptr;
		}
	}

	// If no name or id defined, send back the entity who called the method
	return entity;
}

