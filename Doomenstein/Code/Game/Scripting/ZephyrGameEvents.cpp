#include "Game/Scripting/ZephyrGameEvents.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Framework/Entity.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"
#include "Game/GameEntity.hpp"
#include "Game/SpriteAnimationSetDefinition.hpp"


#define REGISTER_EVENT( eventName ) {\
										m_registeredMethods.insert( #eventName );\
										g_eventSystem->RegisterMethodEvent( #eventName, "", EVERYWHERE, this, &ZephyrGameEvents::eventName );\
									}

//-----------------------------------------------------------------------------------------------
ZephyrGameEvents::ZephyrGameEvents()
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
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );
	if ( targetEntity == nullptr )
	{
		return;
	}

	if ( targetEntity != nullptr )
	{
		targetEntity->Die();
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::Die( EventArgs* args )
{
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );
	if ( targetEntity == nullptr )
	{
		return;
	}

	if ( targetEntity != nullptr )
	{
		targetEntity->Die();
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::WarpToMap( EventArgs* args )
{
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );
	if ( targetEntity == nullptr )
	{
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
void ZephyrGameEvents::RotateEntity( EventArgs* args )
{
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );
	if ( targetEntity == nullptr )
	{
		return;
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
void ZephyrGameEvents::MoveInDirection( EventArgs* args )
{
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );
	if ( targetEntity == nullptr )
	{
		return;
	}

	float speed = args->GetValue( "speed", 1.f );
	Vec3 direction = args->GetValue( "direction", Vec3::ZERO );

	targetEntity->MoveInDirection( speed, direction );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::MoveInRelativeDirection( EventArgs* args )
{
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );
	if ( targetEntity == nullptr )
	{
		return;
	}
	
	float speed = args->GetValue( "speed", 1.f );
	Vec3 direction = args->GetValue( "direction", Vec3::ZERO );

	targetEntity->MoveInRelativeDirection( speed, direction );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::AddImpulse( EventArgs* args )
{
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );
	if ( targetEntity == nullptr )
	{
		return;
	}

	Vec3 impulse = args->GetValue( "impulse", Vec3::ZERO );

	targetEntity->AddImpulse( impulse );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::GetEntityFromCameraRaytrace( EventArgs* args )
{
	float maxDist = args->GetValue( "maxDist", 1.f );

	GameEntity* foundEntity = g_game->GetEntityFromCameraRaycast( maxDist );

	if ( foundEntity != nullptr )
	{
		args->SetValue( "foundEntity", foundEntity->GetId() );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::GetEntityFromRaytrace( EventArgs* args )
{
	UNUSED( args );
	/*Entity* entity = GetTargetEntityFromArgs( args );

	Vec3 startPos = args->GetValue( "startPos", entity->GetPosition() );
	Vec3 forwardVector = args->GetValue( "forwardVector", entity->GetForwardVector() );
	float maxDist = args->GetValue( "maxDist", 1.f );*/


}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::MoveInCircle( EventArgs* args )
{
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );
	if ( targetEntity == nullptr )
	{
		return;
	}

	//float speed = args->GetValue( "speed", .5f );
	//float radius = args->GetValue( "radius", .5f );
	//Vec3 center = args->GetValue( "center", targetEntity->GetPosition() );

	//targetEntity->MoveInCircle( center, radius, speed );
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
void ZephyrGameEvents::ChangeZephyrScriptState( EventArgs* args )
{
	std::string targetState = args->GetValue( "targetState", "" );
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );
	if ( targetEntity == nullptr )
	{
		return;
	}

	if ( targetEntity != nullptr
		 && !targetState.empty() )
	{
		targetEntity->ChangeZephyrScriptState( targetState );
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
void ZephyrGameEvents::PrintDebugText( EventArgs* args )
{
	std::string text = args->GetValue( "text", "TestPrint" );
	float duration = args->GetValue( "duration", 0.f );
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );
	if ( targetEntity == nullptr )
	{
		return;
	}

	std::string colorStr = args->GetValue( "color", "white" );

	Rgba8 color = Rgba8::WHITE;
	if ( colorStr == "white" )		{ color = Rgba8::WHITE; }
	else if ( colorStr == "red" )	{ color = Rgba8::RED; }
	else if ( colorStr == "green" ) { color = Rgba8::GREEN; }
	else if ( colorStr == "blue" )  { color = Rgba8::BLUE; }
	else if ( colorStr == "black" ) { color = Rgba8::BLACK; }

	Mat44 textLocation;

	if ( targetEntity != nullptr )
	{
		// TODO: TransformComponent refactor
		//textLocation.SetTranslation2D( entity->GetPosition().XY() );
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
void ZephyrGameEvents::WinGame( EventArgs* args )
{
	UNUSED( args );

	//g_game->ChangeGameState( eGameState::VICTORY );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::RegisterKeyEvent( EventArgs* args )
{
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );
	if ( targetEntity == nullptr )
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

	targetEntity->RegisterKeyEvent( key, event );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::UnRegisterKeyEvent( EventArgs* args )
{
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );
	if ( targetEntity == nullptr )
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

	targetEntity->UnRegisterKeyEvent( key, event );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::ChangeSpriteAnimation( EventArgs* args )
{
	std::string newAnim = args->GetValue( "newAnim", "" );
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );
	if ( targetEntity == nullptr
		 || newAnim.empty() )
	{
		return;
	}

	targetEntity->ChangeSpriteAnimation( newAnim );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::PlaySpriteAnimation( EventArgs* args )
{
	std::string newAnim = args->GetValue( "newAnim", "" );
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );

	if ( targetEntity == nullptr
		|| newAnim.empty() )
	{
		return;
	}

	targetEntity->PlaySpriteAnimation( newAnim );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::AddAnimationEvent( EventArgs* args )
{
	GameEntity* targetEntity = GetTargetEntityFromArgs( args );
	if ( targetEntity == nullptr )
	{
		return;
	}

	std::string animName = args->GetValue( "anim", "" );
	std::string frameStr = args->GetValue( "frame", "first" );
	std::string eventName = args->GetValue( "event", "" );

	if ( targetEntity == nullptr
		|| animName.empty()
		|| eventName.empty() )
	{
		// print warning
		return;
	}

	SpriteAnimationSetDefinition* spriteAnimSetDef = targetEntity->GetSpriteAnimSetDef( animName );
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
void ZephyrGameEvents::ChangeLight( EventArgs* args )
{
	UNUSED( args );

	//Entity* entity = GetTargetEntityFromArgs( args );


}


//-----------------------------------------------------------------------------------------------
void ZephyrGameEvents::PushCamera( EventArgs* args )
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
void ZephyrGameEvents::PopCamera( EventArgs* args )
{
	UNUSED( args );

	g_game->PopCamera();
}


//-----------------------------------------------------------------------------------------------
GameEntity* ZephyrGameEvents::GetTargetEntityFromArgs( EventArgs* args )
{
	EntityId targetId = (EntityId)args->GetValue( TARGET_ENTITY_STR, INVALID_ENTITY_ID );
	std::string targetName = args->GetValue( TARGET_ENTITY_NAME_STR, "" );

	EntityId entityId = args->GetValue( PARENT_ENTITY_ID_STR, INVALID_ENTITY_ID );
	GameEntity* entity = (GameEntity*)g_game->GetEntityById( entityId );
	if ( entity == nullptr )
	{
		//g_devConsole->PrintError( Stringf( "Parent entity '%s' does not exist in game", Entity::GetName( entityId ).c_str() ) );
		return nullptr;
	}

	// Named entities are returned first
	if ( !targetName.empty() )
	{
		entity = (GameEntity*)g_game->GetEntityByName( targetName );
		if ( entity == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Failed to find an entity with name '%s'", targetName.c_str() ) );
			return nullptr;
		}
	}
	// Id entities are tried next
	else if ( targetId != INVALID_ENTITY_ID )
	{
		entity = (GameEntity*)g_game->GetEntityById( targetId );
		if ( entity == nullptr )
		{
			g_devConsole->PrintWarning( Stringf( "Failed to find an entity with id '%i'", targetId ) );
			return nullptr;
		}
	}

	// If no name or id defined, send back the entity who called the method
	return entity;
}

