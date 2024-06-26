#include "Game/Scripting/ZephyrGameAPI.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
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
	REGISTER_EVENT( StartDialogue );
	REGISTER_EVENT( EndDialogue );
	REGISTER_EVENT( AddLineOfDialogueText );
	REGISTER_EVENT( AddDialogueChoice );
	REGISTER_EVENT( StartNewTimer );
	REGISTER_EVENT( WinGame );

	REGISTER_EVENT( MoveToLocation );
	REGISTER_EVENT( MoveInDirection );
	REGISTER_EVENT( GetEntityLocation );
	REGISTER_EVENT( CheckForTarget );
	REGISTER_EVENT( GetNewWanderTargetPosition );
	REGISTER_EVENT( GetDistanceToTarget );

	REGISTER_EVENT( SpawnEntity );
	REGISTER_EVENT( DamageEntity );
	REGISTER_EVENT( ActivateInvincibility );
	REGISTER_EVENT( DeactivateInvincibility );

	REGISTER_EVENT( RegisterKeyEvent );
	REGISTER_EVENT( UnRegisterKeyEvent );

	REGISTER_EVENT( ChangeSpriteAnimation );
	REGISTER_EVENT( PlaySound );
	REGISTER_EVENT( ChangeMusic );
	REGISTER_EVENT( AddScreenShake );

	REGISTER_EVENT( SpawnEntitiesInRange );
	REGISTER_EVENT( AddAnimationEvent );
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
void ZephyrGameAPI::DamageEntity( EventArgs* args )
{
	EntityId targetId = args->GetValue( "target", (EntityId)-1 );
	Entity* targetEntity = g_game->GetEntityById( targetId );

	float damage = args->GetValue( "damage", 0.f );
	std::string damageType = args->GetValue( "damageType", "normal" );

	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( targetEntity != nullptr
		 && targetEntity->GetFaction() != entity->GetFaction() )
	{
		targetEntity->TakeDamage( damage, damageType );
	}
}


//-----------------------------------------------------------------------------------------------
/**
 * Makes target entity invincible to all damage. 
 * 
 * Target will be determined by the following optional parameters, checking in order the targetId, then targetName, then ( if neither name or id are specified ) targeting the entity who called this event. 
 * params:
 *	- targetId: id of target entity
 *		- Zephyr type: Number
 *	- targetName: name of target entity
 *		- Zephyr type: String
*/
//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::ActivateInvincibility( EventArgs* args )
{
	Entity* entity = GetTargetEntityFromArgs( args );
	if ( entity == nullptr )
	{
		return;
	}

	entity->MakeInvincibleToAllDamage();
}


//-----------------------------------------------------------------------------------------------
/**
 * Reset target entity's damage multipliers.
 *
 * Target will be determined by the following optional parameters, checking in order the targetId, then targetName, then ( if neither name or id are specified ) targeting the entity who called this event.
 * params:
 *	- targetId: id of target entity
 *		- Zephyr type: Number
 *	- targetName: name of target entity
 *		- Zephyr type: String
*/
//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::DeactivateInvincibility( EventArgs* args )
{
	Entity* entity = GetTargetEntityFromArgs( args );
	if ( entity == nullptr )
	{
		return;
	}

	entity->ResetDamageMultipliers();
}


//-----------------------------------------------------------------------------------------------
/**
 * Change the game state to Dialogue to initiate a dialogue with an NPC.
 */
//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::StartDialogue( EventArgs* args )
{
	UNUSED( args );

	g_game->ChangeGameState( eGameState::DIALOGUE );
}


//-----------------------------------------------------------------------------------------------
/**
 * Change the game state back to Playing to end a dialogue sequence.
 */
//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::EndDialogue( EventArgs* args )
{
	UNUSED( args );

	g_game->ChangeGameState( eGameState::PLAYING );
}


//-----------------------------------------------------------------------------------------------
/**
 * Add a line of text to the current dialogue box. 
 *	- Note: Only works if StartDialogue has been called first.
 *
 * params:
 *	- text: Text to add to dialogue box
 *		- Zephyr type: String
 */
//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::AddLineOfDialogueText( EventArgs* args )
{
	std::string text = args->GetValue( "text", "" );

	g_game->AddLineOfDialogueText( text );
}


//-----------------------------------------------------------------------------------------------
/**
 * Add a player selectable choice to the current dialogue box. The player can then select choices with the wasd or arrow keys and confirm with space/enter.
 *	- Note: Only works if StartDialogue has been called first.
 *
 * params:
 *	- name: Name to identify this choice in order to assign a callback event on selection
 *		- Zephyr type: String
 *		- A script can listen for the event "PlayerInteracted" which has a "String choiceName" parameter containing the name of the choice that was selected
 *	
 *	- text: Text to add for choice to dialogue box
 *		- Zephyr type: String
 */
//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::AddDialogueChoice( EventArgs* args )
{
	std::string name = args->GetValue( "name", "missing name" );
	std::string text = args->GetValue( "text", "" );

	g_game->AddDialogueChoice( name, text );
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


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::SpawnEntity( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
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
	float orientation = args->GetValue( "orientation", entity->GetOrientationDegrees() );

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

	Entity* newEntity = mapToSpawnIn->SpawnNewEntityOfTypeAtPosition( entityType, position );
	newEntity->SetOrientationDegrees( orientation );
	newEntity->SetName( name );

	g_game->SaveEntityByName( newEntity );
	
	newEntity->InitializeZephyrEntityVariables();
	newEntity->FireSpawnEvent();

	if ( mapToSpawnIn == g_game->GetCurrentMap() )
	{
		newEntity->Load();
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::WinGame( EventArgs* args )
{
	UNUSED( args );

	g_game->ChangeGameState( eGameState::VICTORY );
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
void ZephyrGameAPI::MoveToLocation( EventArgs* args )
{
	Vec3 targetPos = args->GetValue( "pos", Vec3::ZERO );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity == nullptr )
	{
		return;
	}

	Vec3 moveDirection = targetPos - entity->GetPosition();
	moveDirection.Normalize();

	//float speed = args->GetValue( "speed", entity->GetSpeed() );

	//entity->MoveWithPhysics( speed, moveDirection );
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
void ZephyrGameAPI::MoveInDirection( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
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


//-----------------------------------------------------------------------------------------------
/**
 * Gets the position of the target entity. 
 * 
 * returns:
 *	- Fires the EntityLocationUpdated event with a parameter "entityPos" containing a Vec2 with the requested position
 *
 * params:
 *	Target will be determined by the following optional parameters, checking in order the targetId, then targetName, then ( if neither name or id are specified ) targeting the entity who called this event.
 *	- targetId: id of target entity
 *		- Zephyr type: Number
 *	- targetName: name of target entity
 *		- Zephyr type: String
*/
//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::GetEntityLocation( EventArgs* args )
{
	EntityId targetId = args->GetValue( "target", (EntityId)-1 );
	Entity* targetEntity = g_game->GetEntityById( targetId );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity == nullptr
		 || targetEntity == nullptr )
	{
		return;
	}

	EventArgs targetArgs;
	targetArgs.SetValue( "entityPos", targetEntity->GetPosition() );

	entity->FireScriptEvent( "EntityLocationUpdated", &targetArgs );
}


//-----------------------------------------------------------------------------------------------
/**
 * Gets a new random position inside current map for the calling entity to wander towards.
 *
 * returns:
 *	- Fires the TargetPositionUpdated event with a parameter "newPos" containing a Vec2 with the requested position
 *
 * params:
 *	Target will be determined by the following optional parameters, checking in order the targetId, then targetName, then ( if neither name or id are specified ) targeting the entity who called this event.
 *	- targetId: id of target entity
 *		- Zephyr type: Number
 *	- targetName: name of target entity
 *		- Zephyr type: String
*/
//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::GetNewWanderTargetPosition( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
	if ( entity == nullptr )
	{
		return;
	}

	Map* map = entity->GetMap();
	if ( map == nullptr )
	{
		return;
	}

	Vec2 mapDimensions = map->GetDimensions();
	float newX = g_game->m_rng->RollRandomFloatInRange( 2.f, mapDimensions.x - 2.f );
	float newY = g_game->m_rng->RollRandomFloatInRange( 2.f, mapDimensions.y - 2.f );
	
	args->SetValue( "newPos", Vec3( newX, newY, 0.f ) );

	//entity->FireScriptEvent( "TargetPositionUpdated", &targetArgs );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::CheckForTarget( EventArgs* args )
{
	float maxDist = args->GetValue( "maxDist", 0.f );
	//Entity* targetEntity = GetTargetEntityFromArgs( args );
	
	EntityId targetId = args->GetValue( "target", (EntityId)-1 );
	Entity* targetEntity = g_game->GetEntityById( targetId );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity == nullptr
		 || targetEntity == nullptr )
	{
		return;
	}

	Vec2 displacement = targetEntity->GetPosition().XY() - entity->GetPosition().XY();
	float distBetween = displacement.GetLength();
	
	args->SetValue( "targetFound", distBetween < maxDist );
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::GetDistanceToTarget( EventArgs* args )
{
	EntityId targetId = args->GetValue( "target", (EntityId)-1 );
	Entity* targetEntity = g_game->GetEntityById( targetId );
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity == nullptr
		 || targetEntity == nullptr )
	{
		return;
	}

	Vec2 displacementToTarget = targetEntity->GetPosition().XY() - entity->GetPosition().XY();

	args->SetValue( "distance", displacementToTarget.GetLength() );
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
	//Entity* targetEntity = g_game->GetEntityByName( targetId );
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
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );

	if ( entity == nullptr
		 || newAnim.empty() )
	{
		return;
	}

	entity->PlaySpriteAnimation( newAnim );
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
void ZephyrGameAPI::SpawnEntitiesInRange( EventArgs* args )
{
	Vec2 minPos = args->GetValue( "minPos", Vec2::ZERO );
	Vec2 maxPos = args->GetValue( "maxPos", Vec2::ONE );
	std::string entityType = args->GetValue( "type", "" );
	float entityCount = args->GetValue( "count", 1.f );

	if ( entityType.empty() )
	{
		return;
	}

	for ( int i = 0; i < entityCount; ++i )
	{
		Vec2 randomPosition = Vec2( g_game->m_rng->RollRandomFloatInRange( minPos.x, maxPos.x ), g_game->m_rng->RollRandomFloatInRange( minPos.y, maxPos.y ) );
		args->SetValue( "position", randomPosition );
		g_eventSystem->FireEvent( "SpawnEntity", args );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrGameAPI::AddAnimationEvent( EventArgs* args )
{
	Entity* entity = (Entity*)args->GetValue( "entity", ( void* )nullptr );
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

