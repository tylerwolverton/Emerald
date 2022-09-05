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

	Unload();

	m_curSpriteAnimSetDef = m_entityDef.GetDefaultSpriteAnimSetDef();
}


//-----------------------------------------------------------------------------------------------
Entity::~Entity()
{
	g_eventSystem->DeRegisterObject( this );
}


//-----------------------------------------------------------------------------------------------
void Entity::Update( float deltaSeconds )
{
	m_cumulativeTime += deltaSeconds;

	ZephyrEntity::Update( deltaSeconds );

	UpdateFromKeyboard( deltaSeconds );

	SpriteAnimDefinition* animDef = nullptr;
	if ( m_curSpriteAnimSetDef != nullptr )
	{
		animDef = m_curSpriteAnimSetDef->GetSpriteAnimationDefForDirection( m_facingDirection );
		int frameIndex = animDef->GetFrameIndexAtTime( m_cumulativeTime );

		m_curSpriteAnimSetDef->FireFrameEvent( frameIndex, this );
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::UpdateFromKeyboard(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (g_devConsole->IsOpen())
	{
		return;
	}

	switch (g_game->GetGameState())
	{
		case eGameState::PLAYING:
		{
			for (auto& registeredKey : m_registeredKeyEvents)
			{
				if (g_inputSystem->IsKeyPressed(registeredKey.first))
				{
					for (auto& eventName : registeredKey.second)
					{
						EventArgs args;
						FireScriptEvent(eventName, &args);
					}
				}
			}

			if (g_inputSystem->ConsumeAllKeyPresses(KEY_ENTER)
				|| g_inputSystem->ConsumeAllKeyPresses(KEY_SPACEBAR))
			{
				Vec2 testPoint = GetPosition().XY() + m_forwardVector;
				Entity* targetEntity = m_map->GetEntityAtPosition(testPoint);
				if (targetEntity != nullptr)
				{
					EventArgs args;
					targetEntity->FireScriptEvent("OnPlayerInteract", &args);
				}
			}
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void Entity::Render() const
{
	SpriteAnimDefinition* animDef = nullptr;
	if ( m_curSpriteAnimSetDef == nullptr )
	{
		return;
	}

	animDef = m_curSpriteAnimSetDef->GetSpriteAnimationDefForDirection( Vec2::ZERO );
	
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
	DrawRing2D( g_renderer, GetPosition().XY(), .5f, Rgba8::CYAN, DEBUG_LINE_THICKNESS );
	DrawAABB2Outline( g_renderer, GetPosition().XY(), m_entityDef.m_localDrawBounds, Rgba8::MAGENTA, DEBUG_LINE_THICKNESS );
	//DrawRing2D( g_renderer, GetPosition().XY() + m_forwardVector * ( GetPhysicsRadius() + .1f ), .1f, Rgba8::GREEN, DEBUG_LINE_THICKNESS );
}


//-----------------------------------------------------------------------------------------------
const Vec2 Entity::GetForwardVector() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees );
}


//-----------------------------------------------------------------------------------------------
const Vec3 Entity::GetPosition() const
{
	return m_position;
}


//-----------------------------------------------------------------------------------------------
void Entity::SetPosition( const Vec3& position )
{
	m_position = position;

	EventArgs args;
	args.SetValue( "newPos", m_position );

	FireScriptEvent( "PositionUpdated", &args );
}


//-----------------------------------------------------------------------------------------------
void Entity::SetAsPlayer()
{
	m_isPlayer = true;
	m_name = "player";
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

	if ( varName == "position" ) { m_position = value.GetAsVec3(); return; }

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
}


//-----------------------------------------------------------------------------------------------
void Entity::Unload()
{
}


//-----------------------------------------------------------------------------------------------
void Entity::ChangeSpriteAnimation( const std::string& spriteAnimDefSetName )
{
	SpriteAnimationSetDefinition* newSpriteAnimSetDef = m_entityDef.GetSpriteAnimSetDef( spriteAnimDefSetName );

	if ( newSpriteAnimSetDef == nullptr )
	{
		//g_devConsole->PrintWarning( Stringf( "Warning: Failed to change animation for entity '%s' to undefined animation '%s'", m_name.c_str(), spriteAnimDefSetName.c_str() ) );
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

