#include "Game/Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/2D/DiscCollider.hpp"
#include "Engine/Physics/PhysicsScene.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/ZephyrCore/ZephyrScriptDefinition.hpp"
#include "Engine/ZephyrCore/ZephyrScript.hpp"

#include "Game/EntityDefinition.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/SpriteAnimationSetDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Entity::Entity( const EntityDefinition& entityDef, Map* map )
	: ZephyrEntity( entityDef )
	, m_entityDef( entityDef )
	, m_map( map )
{
	m_curHealth = m_entityDef.GetMaxHealth();

	if ( map != nullptr )
	{
		InitPhysics( map->m_physicsScene->CreateRigidbody() );
	}

	Unload();

	m_curSpriteAnimSetDef = m_entityDef.GetDefaultSpriteAnimSetDef();
}


//-----------------------------------------------------------------------------------------------
Entity::~Entity()
{
	if ( m_rigidbody != nullptr )
	{
		if ( m_entityDef.IsTrigger() )
		{
			m_rigidbody->GetCollider()->m_onTriggerEnterDelegate.UnsubscribeAllMethodsFromObject( this );
			m_rigidbody->GetCollider()->m_onTriggerStayDelegate.UnsubscribeAllMethodsFromObject( this );
			m_rigidbody->GetCollider()->m_onTriggerLeaveDelegate.UnsubscribeAllMethodsFromObject( this );
		}
		else
		{
			m_rigidbody->GetCollider()->m_onOverlapEnterDelegate.UnsubscribeAllMethodsFromObject( this );
			m_rigidbody->GetCollider()->m_onOverlapStayDelegate.UnsubscribeAllMethodsFromObject( this );
			m_rigidbody->GetCollider()->m_onOverlapLeaveDelegate.UnsubscribeAllMethodsFromObject( this );
		}

		m_rigidbody->Destroy();
		m_rigidbody = nullptr;
	}

	g_eventSystem->DeRegisterObject( this );

	PTR_VECTOR_SAFE_DELETE( m_inventory );
}


//-----------------------------------------------------------------------------------------------
void Entity::InitPhysics( Rigidbody* newRigidbody )
{
	if ( m_rigidbody != nullptr )
	{
		m_rigidbody->Destroy();
	}

	m_rigidbody = newRigidbody;

	m_rigidbody->SetMass( m_entityDef.GetMass() );

	m_rigidbody->SetDrag( m_entityDef.GetDrag() );

	std::string collisionLayer = m_entityDef.GetInitialCollisionLayer();
	m_rigidbody->SetLayer( collisionLayer );

	if ( IsEqualIgnoreCase( collisionLayer, "environment" ) )
	{
		m_rigidbody->SetSimulationMode( SIMULATION_MODE_STATIC );
	}
	else if ( IsEqualIgnoreCase( collisionLayer, "npc" ) )
	{
		m_rigidbody->SetSimulationMode( SIMULATION_MODE_KINEMATIC );
	}
	else
	{
		m_rigidbody->SetSimulationMode( SIMULATION_MODE_DYNAMIC );
	}

	if ( m_entityDef.GetSimMode() != eSimulationMode::SIMULATION_MODE_NONE )
	{
		m_rigidbody->SetSimulationMode( m_entityDef.GetSimMode() );
	}

	m_rigidbody->m_userProperties.SetValue( "entityId", m_id );

	NamedProperties params;
	params.SetValue( "radius", GetPhysicsRadius() );

	if ( m_entityDef.IsTrigger() )
	{
		Collider* discTrigger = m_rigidbody->GetPhysicsScene()->CreateTrigger( "disc", &params );

		discTrigger->m_onTriggerEnterDelegate.SubscribeMethod( this, &Entity::EnterTriggerEvent );
		discTrigger->m_onTriggerStayDelegate.SubscribeMethod( this, &Entity::StayTriggerEvent );
		discTrigger->m_onTriggerLeaveDelegate.SubscribeMethod( this, &Entity::ExitTriggerEvent );

		m_rigidbody->TakeCollider( discTrigger );
	}
	else
	{
		Collider* discCollider = m_rigidbody->GetPhysicsScene()->CreateCollider( "disc", &params );

		discCollider->m_onOverlapEnterDelegate.SubscribeMethod( this, &Entity::EnterCollisionEvent );
		discCollider->m_onOverlapStayDelegate.SubscribeMethod( this, &Entity::StayCollisionEvent );
		discCollider->m_onOverlapLeaveDelegate.SubscribeMethod( this, &Entity::ExitCollisionEvent );

		m_rigidbody->TakeCollider( discCollider );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
	m_cumulativeTime += deltaSeconds;
	m_lastDeltaSeconds = deltaSeconds;

	ZephyrEntity::Update( deltaSeconds );

	SpriteAnimDefinition* animDef = nullptr;
	if ( m_curSpriteAnimSetDef != nullptr )
	{
		animDef = m_curSpriteAnimSetDef->GetSpriteAnimationDefForDirection( m_facingDirection );
		int frameIndex = animDef->GetFrameIndexAtTime( m_cumulativeTime );

		m_curSpriteAnimSetDef->FireFrameEvent( frameIndex, this );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::Render() const
{
	SpriteAnimDefinition* animDef = nullptr;
	if ( m_curSpriteAnimSetDef == nullptr
		 || m_rigidbody == nullptr )
	{
		return;
	}

	animDef = m_curSpriteAnimSetDef->GetSpriteAnimationDefForDirection( m_rigidbody->GetVelocity().XY() );
	
	const SpriteDefinition& spriteDef = animDef->GetSpriteDefAtTime( m_cumulativeTime );

	Vec2 mins, maxs;
	spriteDef.GetUVs( mins, maxs );

	std::vector<Vertex_PCU> vertexes;
	AppendVertsForAABB2D( vertexes, m_entityDef.m_localDrawBounds, Rgba8::WHITE, mins, maxs );

	Vertex_PCU::TransformVertexArray( vertexes, 1.f, 0.f, GetPosition() );

	g_renderer->BindTexture( 0, &( spriteDef.GetTexture() ) );
	g_renderer->DrawVertexArray( vertexes );
}


//-----------------------------------------------------------------------------------------------
void Entity::Die()
{
	if ( IsDead() )
	{
		return;
	}

	m_isDead = true;

	ZephyrEntity::Die();
}


//-----------------------------------------------------------------------------------------------
void Entity::DebugRender() const
{
	g_renderer->BindTexture( 0, nullptr );
	DrawRing2D( g_renderer, GetPosition().XY(), m_entityDef.m_physicsRadius, Rgba8::CYAN, DEBUG_LINE_THICKNESS );
	DrawAABB2Outline( g_renderer, GetPosition().XY(), m_entityDef.m_localDrawBounds, Rgba8::MAGENTA, DEBUG_LINE_THICKNESS );
	DrawRing2D( g_renderer, GetPosition().XY() + m_forwardVector * ( GetPhysicsRadius() + .1f ), .1f, Rgba8::GREEN, DEBUG_LINE_THICKNESS );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Entity::GetForwardVector() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Entity::GetPosition() const
{
	if ( m_rigidbody != nullptr )
	{
		return m_rigidbody->GetWorldPosition();
	}

	return Vec3::ZERO;
}


//-----------------------------------------------------------------------------------------------
void Entity::SetPosition( const Vec3& position )
{
	if ( m_rigidbody != nullptr )
	{
		m_rigidbody->SetPosition( position );

		EventArgs args;
		args.SetValue( "newPos", m_rigidbody->GetWorldPosition() );

		FireScriptEvent( "PositionUpdated", &args );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::SetCollisionLayer( uint layer )
{
	if ( m_rigidbody != nullptr )
	{
		m_rigidbody->SetLayer( layer );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::AddItemToInventory( Entity* item )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] == nullptr )
		{
			m_inventory[itemIdx] = item;
			return;
		}
	}

	m_inventory.push_back( item );

	EventArgs args;
	args.SetValue( "itemName", item->GetName() );

	g_eventSystem->FireEvent( "OnAcquireItem", &args );
}


//-----------------------------------------------------------------------------------------------
void Entity::RemoveItemFromInventory( const std::string& itemType )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] != nullptr
			 && m_inventory[itemIdx]->GetType() == itemType )
		{
			m_inventory[itemIdx] = nullptr;
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::RemoveItemFromInventory( const EntityId& itemId )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] != nullptr
			 && m_inventory[itemIdx]->GetId() == itemId )
		{
			m_inventory[itemIdx] = nullptr;
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
bool Entity::IsInInventory( const EntityId& itemId )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] != nullptr
			 && m_inventory[itemIdx]->GetId() == itemId )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void Entity::RemoveItemFromInventory( Entity* item )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] != nullptr
			 && m_inventory[itemIdx] == item )
		{
			m_inventory[itemIdx] = nullptr;
			return;
		}
	}
}


//-----------------------------------------------------------------------------------------------
bool Entity::IsInInventory( const std::string& itemType )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] != nullptr
			&& m_inventory[itemIdx]->GetType() == itemType )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool Entity::IsInInventory( Entity* item )
{
	for ( int itemIdx = 0; itemIdx < (int)m_inventory.size(); ++itemIdx )
	{
		if ( m_inventory[itemIdx] != nullptr
			&& m_inventory[itemIdx] == item )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void Entity::MakeInvincibleToAllDamage()
{
	m_baseDamageMultiplier = 0.f;
	/*for ( auto& damageMultiplier : m_damageTypeMultipliers )
	{
		damageMultiplier.second.curMultiplier = 0.f;
	}*/
}


//-----------------------------------------------------------------------------------------------
void Entity::ResetDamageMultipliers()
{
	m_baseDamageMultiplier = 1.f;

	for ( auto& damageMultiplier : m_damageTypeMultipliers )
	{
		damageMultiplier.second.curMultiplier = damageMultiplier.second.defaultMultiplier;
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::AddNewDamageMultiplier( const std::string& damageType, float newMultiplier )
{
	m_damageTypeMultipliers[damageType] = DamageMultiplier( newMultiplier );
}


//-----------------------------------------------------------------------------------------------
void Entity::ChangeDamageMultiplier( const std::string& damageType, float newMultiplier )
{
	auto damageIter = m_damageTypeMultipliers.find( damageType );
	if ( damageIter == m_damageTypeMultipliers.end() )
	{
		g_devConsole->PrintError( Stringf( "Tried to change multiplier of unknown damage type '%s'", damageType.c_str() ) );
	}

	damageIter->second.curMultiplier = newMultiplier;
}


//-----------------------------------------------------------------------------------------------
void Entity::PermanentlyChangeDamageMultiplier( const std::string& damageType, float newDefaultMultiplier )
{
	auto damageIter = m_damageTypeMultipliers.find( damageType );
	if ( damageIter == m_damageTypeMultipliers.end() )
	{
		g_devConsole->PrintError( Stringf( "Tried to permanently change multiplier of unknown damage type '%s'", damageType.c_str() ) );
	}

	damageIter->second.defaultMultiplier = newDefaultMultiplier;
	damageIter->second.curMultiplier = newDefaultMultiplier;
}


//-----------------------------------------------------------------------------------------------
void Entity::TakeDamage( float damage, const std::string& type )
{
	if ( IsDead() )
	{
		return;
	}

	float damageMultiplier = 1.f;

	auto damageIter = m_damageTypeMultipliers.find( type );
	if ( damageIter != m_damageTypeMultipliers.end() )
	{
		damageMultiplier = damageIter->second.curMultiplier;
	}

	m_curHealth -= damage * damageMultiplier * m_baseDamageMultiplier;
	if ( m_curHealth <= 0 )
	{
		Die();
	}
	
	if ( m_scriptObj != nullptr )
	{
		EventArgs args;
		args.SetValue( "newHealth", m_curHealth );

		m_scriptObj->FireEvent( "OnHealthChange", &args );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::MoveWithPhysics( float speed, const Vec2& direction )
{
	if ( m_rigidbody != nullptr )
	{
		m_rigidbody->ApplyImpulseAt( speed * Vec3( direction, 0.f ) * m_lastDeltaSeconds, GetPosition() );
		m_forwardVector = direction;

		EventArgs args;
		args.SetValue( "newPos", m_rigidbody->GetWorldPosition() );

		FireScriptEvent( "OnPositionChange", &args );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::EnableRigidbody()
{
	if ( m_rigidbody != nullptr )
	{
		m_rigidbody->Enable();
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::DisableRigidbody()
{
	if ( m_rigidbody != nullptr )
	{
		m_rigidbody->Disable();
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::RegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName )
{
	char keyCode = GetKeyCodeFromString( keyCodeStr );
	
	if ( keyCode == '\0' )
	{
		return;
	}

	m_registeredKeyEvents[keyCode].push_back( eventName );
}


//-----------------------------------------------------------------------------------------------
void Entity::UnRegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName )
{
	char keyCode = GetKeyCodeFromString( keyCodeStr );

	if ( keyCode == '\0' )
	{
		return;
	}

	auto eventIter = m_registeredKeyEvents.find(keyCode);
	if ( eventIter == m_registeredKeyEvents.end() )
	{
		return;
	}

	Strings& eventNames = eventIter->second;
	int eventIdx = 0;
	for ( ; eventIdx < (int)eventNames.size(); ++eventIdx )
	{
		if ( eventName == eventNames[eventIdx] )
		{
			break;
		}
	}

	// Remove bound event
	if ( eventIdx < (int)eventNames.size() )
	{
		eventNames.erase( eventNames.begin() + eventIdx );
	}
}


//-----------------------------------------------------------------------------------------------
ZephyrValue Entity::GetGlobalVariable( const std::string& varName )
{
	if ( !IsScriptValid() )
	{
		return ZephyrValue( ERROR_ZEPHYR_VAL );
	}

	// First check c++ built in vars
	if ( varName == "id" ) { return ZephyrValue( (float)GetId() ); }
	if ( varName == "name" ) { return ZephyrValue( GetName() ); }
	if ( varName == "health" ) { return ZephyrValue( (float)m_curHealth ); }
	if ( varName == "maxHealth" ) { return ZephyrValue( (float)m_entityDef.GetMaxHealth() ); }
	if ( varName == "position" ) { return ZephyrValue( GetPosition() ); }
	if ( varName == "forwardVec" ) { return ZephyrValue( GetForwardVector() ); }
	if ( varName == "speed" ) { return ZephyrValue( GetSpeed() ); }

	return ZephyrEntity::GetGlobalVariable( varName );
}


//-----------------------------------------------------------------------------------------------
void Entity::SetGlobalVariable( const std::string& varName, const ZephyrValue& value )
{
	if ( !IsScriptValid() )
	{
		return;
	}

	// First check c++ built in vars
	if ( varName == "health" )
	{
		m_curHealth = value.GetAsNumber();
		if ( m_curHealth < 0 )
		{
			Die();
		}

		return;
	}

	ZephyrEntity::SetGlobalVariable( varName, value );
}


//-----------------------------------------------------------------------------------------------
void Entity::AddGameEventParams( EventArgs* args ) const
{
	if ( m_map != nullptr )
	{
		args->SetValue( "mapName", m_map->GetName() );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::Load()
{
	if ( m_isDead )
	{
		return;
	}

	m_rigidbody->Enable();
}


//-----------------------------------------------------------------------------------------------
void Entity::Unload()
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->Disable();
}


//-----------------------------------------------------------------------------------------------
void Entity::ChangeSpriteAnimation( const std::string& spriteAnimDefSetName )
{
	SpriteAnimationSetDefinition* newSpriteAnimSetDef = m_entityDef.GetSpriteAnimSetDef( spriteAnimDefSetName );

	if ( newSpriteAnimSetDef == nullptr )
	{
		g_devConsole->PrintWarning( Stringf( "Warning: Failed to change animation for entity '%s' to undefined animation '%s'", m_name.c_str(), spriteAnimDefSetName.c_str() ) );
		return;
	}

	m_curSpriteAnimSetDef = newSpriteAnimSetDef;
}


//-----------------------------------------------------------------------------------------------
void Entity::PlaySpriteAnimation( const std::string& spriteAnimDefSetName )
{
	SpriteAnimationSetDefinition* newSpriteAnimSetDef = m_entityDef.GetSpriteAnimSetDef( spriteAnimDefSetName );

	if ( newSpriteAnimSetDef == nullptr )
	{
		//g_devConsole->PrintWarning( Stringf( "Warning: Failed to change animation for entity '%s' to undefined animation '%s'", GetName().c_str(), spriteAnimDefSetName.c_str() ) );
		return;
	}

	m_curSpriteAnimSetDef = newSpriteAnimSetDef;
	m_cumulativeTime = 0.f;
}


//-----------------------------------------------------------------------------------------------
void Entity::EnterCollisionEvent( Collision collision )
{
	SendPhysicsEventToScript( collision, "OnCollisionEnter" );
}


//-----------------------------------------------------------------------------------------------
void Entity::StayCollisionEvent( Collision collision )
{
	SendPhysicsEventToScript( collision, "OnCollisionStay" );
}


//-----------------------------------------------------------------------------------------------
void Entity::ExitCollisionEvent( Collision collision )
{
	SendPhysicsEventToScript( collision, "OnCollisionExit" );
}


//-----------------------------------------------------------------------------------------------
void Entity::EnterTriggerEvent( Collision collision )
{
	SendPhysicsEventToScript( collision, "OnTriggerEnter" );
}


//-----------------------------------------------------------------------------------------------
void Entity::StayTriggerEvent( Collision collision )
{
	SendPhysicsEventToScript( collision, "OnTriggerStay" );
}


//-----------------------------------------------------------------------------------------------
void Entity::ExitTriggerEvent( Collision collision )
{
	SendPhysicsEventToScript( collision, "OnTriggerExit" );
}


//-----------------------------------------------------------------------------------------------
void Entity::SendPhysicsEventToScript( Collision collision, const std::string& eventName )
{
	if ( !IsDead() )
	{
		//Entity* theirEntity = (Entity*)collision.theirCollider->m_rigidbody->m_userProperties.GetValue( "entityId", (EntityId)-1 );
		EntityId theirEntityId = collision.theirCollider->GetRigidbody()->m_userProperties.GetValue( "entityId", (EntityId)-1 );

		Entity* theirEntity = g_game->GetEntityById( theirEntityId );

		if ( IsScriptValid() )
		{
			// Save data if collision is with another entity
			// Still report collision regardless to account for environmental collisions
			EventArgs args;
			EntityId otherId = -1;
			std::string otherName;
			std::string otherType;
			if ( theirEntity != nullptr
				 && !theirEntity->IsDead()
				 && theirEntity->GetId() > 0 )
			{
				otherId = theirEntity->GetId();
				otherName = theirEntity->GetName();
				otherType = theirEntity->GetType();
			}

			args.SetValue( "otherEntity", otherId );
			args.SetValue( "otherEntityName", otherName );
			args.SetValue( "otherEntityType", otherType );
			m_scriptObj->FireEvent( eventName, &args );
		}
	}
}


//-----------------------------------------------------------------------------------------------
char Entity::GetKeyCodeFromString( const std::string& keyCodeStr )
{
	if ( IsEqualIgnoreCase( keyCodeStr, "space" ) )			{ return KEY_SPACEBAR; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "enter" ) )	{ return KEY_ENTER; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "shift" ) )	{ return KEY_SHIFT; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "left" ) )		{ return KEY_LEFTARROW; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "right" ) )	{ return KEY_RIGHTARROW; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "up" ) )		{ return KEY_UPARROW; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "down" ) )		{ return KEY_DOWNARROW; }
	else
	{
		// Register letters and numbers
		char key = keyCodeStr[0];
		if ( key >= 'a' && key <= 'z' )
		{
			key -= 32;
		}

		if ( ( key >= '0' && key <= '9' )
			 || ( key >= 'A' && key <= 'Z' ) )
		{
			return key;
		}
	}

	return '\0';
}

