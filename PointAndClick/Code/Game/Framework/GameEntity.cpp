#include "Game/Framework/GameEntity.hpp"
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
#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"

#include "Game/Core/GameCommon.hpp"
#include "Game/DataParsing/EntityTypeDefinition.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/Map.hpp"
#include "Game/Graphics/SpriteAnimationSetDefinition.hpp"
#include "Game/Graphics/SpriteAnimationSystem.hpp"


//-----------------------------------------------------------------------------------------------
GameEntity::GameEntity( const EntityTypeDefinition& entityDef, Map* map )
	: m_entityDef( entityDef )
	, m_map( map )
{
	m_curHealth = m_entityDef.GetMaxHealth();

	Unload();
}


//-----------------------------------------------------------------------------------------------
GameEntity::~GameEntity()
{
	g_eventSystem->DeRegisterObject( this );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::UpdateFromKeyboard()
{
	if (g_devConsole->IsOpen())
	{
		return;
	}

	switch (g_game->GetGameState())
	{
		case eGameState::PLAYING:
		{
			// Update script button events
			for (const auto& registeredKey : m_registeredKeyEvents)
			{
				if (g_inputSystem->IsKeyPressed(registeredKey.first))
				{
					for (const auto& eventName : registeredKey.second)
					{
						EventArgs args;
						ZephyrSystem::FireScriptEvent( m_id, eventName, &args);
					}
				}
			}

			/*if (g_inputSystem->ConsumeAllKeyPresses(KEY_ENTER)
				|| g_inputSystem->ConsumeAllKeyPresses(KEY_SPACEBAR))
			{
				Vec2 testPoint = GetPosition().XY() + m_forwardVector;
				GameEntity* targetEntity = m_map->GetEntityAtPosition(testPoint);
				if (targetEntity != nullptr)
				{
					EventArgs args;
					targetEntity->FireScriptEvent("OnPlayerInteract", &args);
				}
			}*/
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void GameEntity::DebugRender() const
{
	g_renderer->BindTexture( 0, nullptr );
	DrawRing2D( g_renderer, GetPosition().XY(), .5f, Rgba8::CYAN, DEBUG_LINE_THICKNESS );
	//DrawAABB2Outline( g_renderer, GetPosition().XY(), m_entityDef.m_localDrawBounds, Rgba8::MAGENTA, DEBUG_LINE_THICKNESS );
	//DrawRing2D( g_renderer, GetPosition().XY() + m_forwardVector * ( GetPhysicsRadius() + .1f ), .1f, Rgba8::GREEN, DEBUG_LINE_THICKNESS );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::Load()
{
	if ( m_isDead )
	{
		return;
	}
}


//-----------------------------------------------------------------------------------------------
void GameEntity::Unload()
{
}


//-----------------------------------------------------------------------------------------------
void GameEntity::Die()
{
	m_isDead = true;
}


//-----------------------------------------------------------------------------------------------
const Vec2 GameEntity::GetForwardVector() const
{
	return Vec2::MakeFromPolarDegrees( m_transform.GetYawDegrees() );
}


//-----------------------------------------------------------------------------------------------
const Vec3 GameEntity::GetPosition() const
{
	return m_transform.GetPosition();
}


//-----------------------------------------------------------------------------------------------
void GameEntity::SetPosition( const Vec3& position )
{
	m_transform.SetPosition( position );

	//EventArgs args;
	//args.SetValue( "newPos", m_position );

	//ZephyrSystem::FireScriptEvent( m_id, "PositionUpdated", &args );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::SetOrientationDegrees( float orientationDegrees )
{
	m_transform.SetOrientationFromPitchRollYawDegrees( 0.f, 0.f, orientationDegrees );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::RegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName )
{
	char keyCode = GetKeyCodeFromString( keyCodeStr );
	
	if ( keyCode == '\0' )
	{
		return;
	}

	m_registeredKeyEvents[keyCode].push_back( eventName );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::UnRegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName )
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
void GameEntity::FireSpawnEvent()
{
	ZephyrSystem::FireSpawnEvent( m_id );
}


//-----------------------------------------------------------------------------------------------
bool GameEntity::FireScriptEvent( const std::string& eventName, EventArgs* args )
{
	return ZephyrSystem::FireScriptEvent( m_id, eventName, args );
}

// TODO: Find a way to handle setting/getting built in c++ vars
//-----------------------------------------------------------------------------------------------
ZephyrValue GameEntity::GetGlobalVariable( const std::string& varName )
{
	// First check c++ built in vars
	if ( varName == "id" )			{ return ZephyrValue( (float)GetId() ); }
	if ( varName == "name" )		{ return ZephyrValue( GetName() ); }
	if ( varName == "health" )		{ return ZephyrValue( (float)m_curHealth ); }
	if ( varName == "maxHealth" )	{ return ZephyrValue( (float)m_entityDef.GetMaxHealth() ); }
	if ( varName == "position" )	{ return ZephyrValue( GetPosition() ); }
	if ( varName == "forwardVec" )	{ return ZephyrValue( GetForwardVector() ); }

	return ZephyrSystem::GetGlobalVariable( m_id, varName );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::SetGlobalVariable( const std::string& varName, const ZephyrValue& value )
{
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

	if ( varName == "position" ) { SetPosition( value.GetAsVec3() ); return; }

	ZephyrSystem::SetGlobalVariable( m_id, varName, value );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::ChangeSpriteAnimation( const std::string& spriteAnimDefSetName )
{
	SpriteAnimationSystem::ChangeSpriteAnimation( m_id, spriteAnimDefSetName );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::PlaySpriteAnimation( const std::string& spriteAnimDefSetName )
{
	SpriteAnimationSystem::PlaySpriteAnimation( m_id, spriteAnimDefSetName );
}


//-----------------------------------------------------------------------------------------------
char GameEntity::GetKeyCodeFromString( const std::string& keyCodeStr )
{
	if ( IsEqualIgnoreCase( keyCodeStr, "space" ) )					{ return KEY_SPACEBAR; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "enter" ) )			{ return KEY_ENTER; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "shift" ) )			{ return KEY_SHIFT; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "left" ) )				{ return KEY_LEFTARROW; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "right" ) )			{ return KEY_RIGHTARROW; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "up" ) )				{ return KEY_UPARROW; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "down" ) )				{ return KEY_DOWNARROW; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "leftClick" ) )		{ return MOUSE_LBUTTON; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "rightClick" ) )		{ return MOUSE_RBUTTON; }
	else if ( IsEqualIgnoreCase( keyCodeStr, "middleClick" ) )		{ return MOUSE_MBUTTON; }
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

