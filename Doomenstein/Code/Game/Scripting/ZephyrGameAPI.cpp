#include "Game/Scripting/ZephyrGameAPI.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/ZephyrCore/ZephyrSystem.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/Entity.hpp"
#include "Game/SpriteAnimationSetDefinition.hpp"


#define REGISTER_EVENT( eventName ) {\
										m_registeredMethods.insert( #eventName );\
										g_eventSystem->RegisterMethodEvent( #eventName, "", EVERYWHERE, this, &ZephyrGameAPI::eventName );\
									}

//-----------------------------------------------------------------------------------------------
ZephyrGameAPI::ZephyrGameAPI()
{
	REGISTER_EVENT( ChangeZephyrScriptState );
	REGISTER_EVENT( PrintDebugText );
	REGISTER_EVENT( PrintDebugScreenText );
	REGISTER_EVENT( PrintToConsole );

	REGISTER_EVENT( DestroySelf );
	REGISTER_EVENT( Die );
	REGISTER_EVENT( WarpToMap );
	REGISTER_EVENT( RotateEntity );
	REGISTER_EVENT( MoveInCircle );
	REGISTER_EVENT( MoveInDirection );
	REGISTER_EVENT( MoveInRelativeDirection );
	REGISTER_EVENT( AddImpulse );

	REGISTER_EVENT( StartNewTimer );
	REGISTER_EVENT( WinGame );
	
	/*REGISTER_EVENT( SpawnEntity );
	REGISTER_EVENT( DamageEntity );*/

	REGISTER_EVENT( RegisterKeyEvent );
	REGISTER_EVENT( UnRegisterKeyEvent );

	REGISTER_EVENT( ChangeSpriteAnimation );
	REGISTER_EVENT( PlaySpriteAnimation );
	REGISTER_EVENT( AddAnimationEvent );
	REGISTER_EVENT( PlaySound );
	REGISTER_EVENT( ChangeMusic );
	REGISTER_EVENT( AddScreenShake );
	REGISTER_EVENT( ChangeLight );
	REGISTER_EVENT( GetEntityFromRaytrace );
	REGISTER_EVENT( GetEntityFromCameraRaytrace );

	REGISTER_EVENT( PushCamera );
	REGISTER_EVENT( PopCamera );
}


//-----------------------------------------------------------------------------------------------
ZephyrGameAPI::~ZephyrGameAPI()
{
}


//-----------------------------------------------------------------------------------------------
ZephyrEntity* ZephyrGameAPI::GetEntityById( const EntityId& id ) const
{
	return g_game->GetEntityById( id );
}


//-----------------------------------------------------------------------------------------------
ZephyrEntity* ZephyrGameAPI::GetEntityByName( const std::string& name ) const
{
	return g_game->GetEntityByName( name );
}


//-----------------------------------------------------------------------------------------------
/**
 * Destroys the entity who called this event.
 */
//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::DestroySelf( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity != nullptr )
	{
		entity->Die();
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::Die( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	entity->Die();
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::WarpToMap( EventArgs* args )
{
	EntityId targetId = args->GetValue( "target", (EntityId)-1 );

	Entity* targetEntity = g_game->GetEntityById( targetId );
	if ( targetEntity == nullptr )
	{
		g_devConsole->PrintWarning( "Tried to warp entity that doesn't exist" );
		return;
	}

	Vec3 newPosition = args->GetValue( "position", targetEntity->GetPosition() );
	float newYawOffset = args->GetValue( "yawOffset", targetEntity->GetOrientationDegrees() );
	std::string destMap = args->GetValue( "map", "" );

	if ( destMap.empty() )
	{
		destMap = targetEntity->GetMap()->GetName();
	}

	g_game->WarpEntityToMap( targetEntity, destMap, newPosition, newYawOffset );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::RotateEntity( EventArgs* args )
{
	EntityId targetId = args->GetValue( "target", (EntityId)-1 );
	
	Entity* targetEntity = g_game->GetEntityById( targetId );
	if ( targetEntity == nullptr )
	{
		targetEntity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	}

	float pitchDegrees = args->GetValue( "pitchDegrees", 0.f );
	float yawDegrees = args->GetValue( "yawDegrees", 0.f );
	float rollDegrees = args->GetValue( "rollDegrees", 0.f );
	
	if ( targetEntity != nullptr )
	{
		targetEntity->RotateDegrees( pitchDegrees, yawDegrees, rollDegrees );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::MoveInDirection( EventArgs* args )
{
	EntityId targetId = args->GetValue( "target", (EntityId)-1 );

	Entity* targetEntity = g_game->GetEntityById( targetId );
	if ( targetEntity == nullptr )
	{
		targetEntity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	}

	float speed = args->GetValue( "speed", 1.f );
	Vec3 direction = args->GetValue( "direction", Vec3::ZERO );

	targetEntity->MoveInDirection( speed, direction );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::MoveInRelativeDirection( EventArgs* args )
{
	EntityId targetId = args->GetValue( "target", (EntityId)-1 );

	Entity* targetEntity = g_game->GetEntityById( targetId );
	if ( targetEntity == nullptr )
	{
		targetEntity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	}

	float speed = args->GetValue( "speed", 1.f );
	Vec3 direction = args->GetValue( "direction", Vec3::ZERO );

	targetEntity->MoveInRelativeDirection( speed, direction );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::AddImpulse( EventArgs* args )
{
	EntityId targetId = args->GetValue( "target", (EntityId)-1 );

	Entity* targetEntity = g_game->GetEntityById( targetId );
	if ( targetEntity == nullptr )
	{
		targetEntity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	}

	Vec3 impulse = args->GetValue( "impulse", Vec3::ZERO );

	targetEntity->AddImpulse( impulse );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::GetEntityFromCameraRaytrace( EventArgs* args )
{
	float maxDist = args->GetValue( "maxDist", 1.f );

	Entity* foundEntity = g_game->GetEntityFromCameraRaycast( maxDist );

	if ( foundEntity != nullptr )
	{
		args->SetValue( "foundEntity", foundEntity->GetId() );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::GetEntityFromRaytrace( EventArgs* args )
{
	UNUSED( args );
	/*Entity* entity = GetTargetEntityFromArgs( args );

	Vec3 startPos = args->GetValue( "startPos", entity->GetPosition() );
	Vec3 forwardVector = args->GetValue( "forwardVector", entity->GetForwardVector() );
	float maxDist = args->GetValue( "maxDist", 1.f );*/


}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::MoveInCircle( EventArgs* args )
{
	EntityId targetId = args->GetValue( "target", (EntityId)-1 );

	Entity* targetEntity = g_game->GetEntityById( targetId );
	if ( targetEntity == nullptr )
	{
		targetEntity = (Entity*)args->GetValue( "entity", (void*)nullptr );
	}

	//float speed = args->GetValue( "speed", .5f );
	//float radius = args->GetValue( "radius", .5f );
	//Vec3 center = args->GetValue( "center", targetEntity->GetPosition() );

	//targetEntity->MoveInCircle( center, radius, speed );
}


//-----------------------------------------------------------------------------------------------
/**
 * Damages target entity. Will not do damage if entity applying damage is in the same faction as the entity to damage.
 * 
 * Target will be determined by the following optional parameters, checking in order the targetId, then targetName, then ( if neither name or id are specified ) targeting the entity who called this event.
 * params:
 *	- targetId: id of target entity
 *		- Zephyr type: Number
 *	- targetName: name of target entity
 *		- Zephyr type: String
 * 
 *	- damage: amount of damage to deal
 *		- Zephyr type: Number
 *	- damageType: type of damage to deal
 *		- Zephyr type: String
 *		- default: "normal"
*/
//-----------------------------------------------------------------------------------------------
//void ZephyrGameAPI::DamageEntity( EventArgs* args )
//{
//	EntityId targetId = args->GetValue( "target", (EntityId)-1 );
//	Entity* targetEntity = g_game->GetEntityById( targetId );
//
//	float damage = args->GetValue( "damage", 0.f );
//	std::string damageType = args->GetValue( "damageType", "normal" );
//
//	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
//
//	if ( targetEntity != nullptr
//		 && targetEntity->GetFaction() != entity->GetFaction() )
//	{
//		targetEntity->TakeDamage( damage, damageType );
//	}
//}


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
void ZephyrGameAPI::StartNewTimer( EventArgs* args )
{
	std::string timerName = args->GetValue( "name", "" );
	float durationSeconds = args->GetValue( "durationSeconds", 1.f );
	std::string onCompletedEventName = args->GetValue( "onCompletedEvent", "" );
	bool broadcastEventToAll = args->GetValue( "broadcastEventToAll", false );
	
	// Broadcast event to all takes precedence and broadcasts to all entities
	if ( broadcastEventToAll )
	{
		g_zephyrSystem->StartNewTimer( -1, timerName, durationSeconds, onCompletedEventName, args );
		return;
	}
	
	Entity* entity = GetTargetEntityFromArgs( args );
	if ( entity == nullptr )
	{
		return;
	}

	g_zephyrSystem->StartNewTimer( entity->GetId(), timerName, durationSeconds, onCompletedEventName, args );
}


//-----------------------------------------------------------------------------------------------
/**
 * Change the current State of the Zephyr script for the entity who called the event.
 *	Note: Called like ChangeState( newState ) in script as a built in function
 *
 * params:
 *	- targetState: the name of the Zephyr State to change to
 *		- Zephyr type: String
*/
//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::ChangeZephyrScriptState( EventArgs* args )
{
	std::string targetState = args->GetValue( "targetState", "" );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity != nullptr
		 && !targetState.empty() )
	{
		entity->ChangeZephyrScriptState( targetState );
	}
}


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
void ZephyrGameAPI::PrintDebugText( EventArgs* args )
{
	std::string text = args->GetValue( "text", "TestPrint" );
	float duration = args->GetValue( "duration", 0.f );
	Entity* entity = (Entity*)args->GetValue( "entity", (void*)nullptr );

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
void ZephyrGameAPI::PrintDebugScreenText( EventArgs* args )
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
void ZephyrGameAPI::PrintToConsole( EventArgs* args )
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

//
////-----------------------------------------------------------------------------------------------
//void ZephyrGameAPI::SpawnEntity( EventArgs* args )
//{
//	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
//	if ( entity == nullptr )
//	{
//		return;
//	}
//
//	std::string entityType = args->GetValue( "type", "" );
//	if ( entityType.empty() )
//	{
//		return;
//	}
//
//	std::string name = args->GetValue( "name", "" );
//	std::string mapName = args->GetValue( "map", "" );
//	Vec2 position = args->GetValue( "position", entity->GetPosition() );
//	float orientation = args->GetValue( "orientation", entity->GetOrientationDegrees() );
//
//	Map* mapToSpawnIn = entity->GetMap();
//	if ( mapToSpawnIn == nullptr )
//	{
//		mapToSpawnIn = g_game->GetCurrentMap();
//	}
//
//	if ( !mapName.empty() )
//	{
//		mapToSpawnIn = g_game->GetMapByName( mapName );
//		if ( mapToSpawnIn == nullptr )
//		{
//			g_devConsole->PrintError( Stringf( "Can't spawn entity in nonexistent map '%s'", mapName.c_str() ) );
//			return;
//		}
//	}
//
//	Entity* newEntity = mapToSpawnIn->SpawnNewEntityOfTypeAtPosition( entityType, position );
//	newEntity->SetOrientationDegrees( orientation );
//	newEntity->SetName( name );
//
//	g_game->SaveEntityByName( newEntity );
//	
//	newEntity->InitializeZephyrEntityVariables();
//	newEntity->FireSpawnEvent();
//
//	if ( mapToSpawnIn == g_game->GetCurrentMap() )
//	{
//		newEntity->Load();
//	}
//}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::WinGame( EventArgs* args )
{
	UNUSED( args );

	//g_game->ChangeGameState( eGameState::VICTORY );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::RegisterKeyEvent( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
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
void ZephyrGameAPI::UnRegisterKeyEvent( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
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
void ZephyrGameAPI::ChangeSpriteAnimation( EventArgs* args )
{
	std::string newAnim = args->GetValue( "newAnim", "" );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity == nullptr
		 || newAnim.empty() )
	{
		return;
	}

	entity->ChangeSpriteAnimation( newAnim );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::PlaySpriteAnimation( EventArgs* args )
{
	std::string newAnim = args->GetValue( "newAnim", "" );
	//Entity* targetEntity = g_game->GetEntityByName( targetId );
	Entity* entity = (Entity*)args->GetValue( "entity", (void*)nullptr );

	if ( entity == nullptr
		|| newAnim.empty() )
	{
		return;
	}

	entity->PlaySpriteAnimation( newAnim );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::AddAnimationEvent( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", (void*)nullptr );
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

	SpriteAnimationSetDefinition* spriteAnimSetDef = entity->GetSpriteAnimSetDef( animName );
	if ( spriteAnimSetDef == nullptr )
	{
		// Print error
		return;
	}

	int numFrames = spriteAnimSetDef->GetNumFrames();
	if ( numFrames == 0 )
	{
		// print error
		return;
	}

	int frameNum = 0;
	if ( frameStr == "first" )
	{
		frameNum = 0;
	}
	else if ( frameStr == "last" )
	{
		frameNum = numFrames - 1;
	}
	else
	{
		frameNum = FromString( frameStr, frameNum );
	}

	if ( frameNum >= numFrames )
	{
		// print warning
		frameNum = numFrames - 1;
	}

	spriteAnimSetDef->AddFrameEvent( frameNum, eventName );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::PlaySound( EventArgs* args )
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
void ZephyrGameAPI::ChangeMusic( EventArgs* args )
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
void ZephyrGameAPI::AddScreenShake( EventArgs* args )
{
	float intensity = args->GetValue( "intensity", 0.f );

	g_game->AddScreenShakeIntensity( intensity );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::ChangeLight( EventArgs* args )
{
	UNUSED( args );

	//Entity* entity = GetTargetEntityFromArgs( args );


}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::PushCamera( EventArgs* args )
{
	std::string type = args->GetValue( "type", "fps" );
	float fov = args->GetValue( "fovDegrees", 90.f );

	GameCameraSettings gameCameraSettings;
	gameCameraSettings.fovDegrees = fov;

	if ( IsEqualIgnoreCase( type, "fps" ) )
	{
		gameCameraSettings.gameCameraType = eGameCameraType::FPS;
	}
	else
	{
		gameCameraSettings.gameCameraType = eGameCameraType::FREE;
	}

	g_game->PushCamera( gameCameraSettings );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::PopCamera( EventArgs* args )
{
	UNUSED( args );

	g_game->PopCamera();
}


//-----------------------------------------------------------------------------------------------
Entity* ZephyrGameAPI::GetTargetEntityFromArgs( EventArgs* args )
{
	EntityId targetId = (EntityId)args->GetValue( "targetId", -1.f );
	std::string targetName = args->GetValue( "targetName", "" );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

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

