#include "Game/GameEntity.hpp"
#include "Game/EntityDefinition.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/SpriteAnimationSetDefinition.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
GameEntity::GameEntity( const EntityDefinition& entityDef, Map* map )
	: m_entityDef( entityDef )
	, m_map( map )
{

	m_gameLight.light.intensity				= m_entityDef.m_lightIntensity;
	m_gameLight.light.color					= m_entityDef.m_lightColor;
	m_gameLight.light.attenuation			= m_entityDef.m_lightAttenuation;
	m_gameLight.light.specularAttenuation	= m_entityDef.m_lightSpecularAttenuation;
	m_gameLight.light.halfCosOfInnerAngle	= m_entityDef.m_lightHalfCosOfInnerAngle;
	m_gameLight.light.halfCosOfOuterAngle	= m_entityDef.m_lightHalfCosOfOuterAngle;
	m_gameLight.isEnabled					= m_entityDef.m_isLightEnabled;

	//m_collisionCenterOffset = Transform::GetWorldUpVector() * m_entityDef.m_physicsRadius;

	m_curSpriteAnimSetDef = m_entityDef.GetDefaultSpriteAnimSetDef();

	//m_rigidbody = g_game->GetPhysicsSystem()->CreateRigidbody( m_entityDef.m_mass );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::Update( float deltaSeconds )
{
	m_cumulativeTime += deltaSeconds;

	// REFACTOR: Move to input component
	if ( !m_isDead )
	{
		if ( m_isPossessed )
		{
			UpdateFromKeyboard( deltaSeconds );
		}
	}

	//ZephyrEntity::Update( deltaSeconds );

	// REFACTOR: Move to GraphicsComponent/SpriteAnimationComponent
	Vec3 position = GetPosition();
	if ( m_curSpriteAnimSetDef != nullptr )
	{
		SpriteAnimDefinition* animDef = m_curSpriteAnimSetDef->GetSpriteAnimationDefForDirection( position.XY(), m_orientationDegrees, *g_game->GetWorldCamera() );
		int frameIndex = animDef->GetFrameIndexAtTime( m_cumulativeTime );

		m_curSpriteAnimSetDef->FireFrameEvent( frameIndex, this );
	}

	// REFACTOR: Do this better somehow
	switch ( m_gameLight.type )
	{
		case eLightType::DYNAMIC_LIGHT:
		{
			m_gameLight.light.position.x = position.x;
			m_gameLight.light.position.z = position.y;
			m_gameLight.light.direction = Vec3( GetForwardVector(), 0.f ); // TODO: Do this properly
		}
		break;
	}

	if ( m_gameLight.isEnabled )
	{
		g_game->AcquireAndSetLightFromPool( m_gameLight.light );
	}
}


//-----------------------------------------------------------------------------------------------
void GameEntity::Render() const
{
	if ( m_isPossessed 
		|| m_curSpriteAnimSetDef == nullptr )
	{
		return;
	}

	// Billboarding
	Vec3 corners[4];
	Vec3 position = GetPosition();
	switch ( m_entityDef.m_billboardStyle )
	{
		case eBillboardStyle::CAMERA_FACING_XY:		BillboardSpriteCameraFacingXY( position, m_entityDef.GetVisualSize(), *g_game->GetWorldCamera(), corners );	 break;
		case eBillboardStyle::CAMERA_OPPOSING_XY:	BillboardSpriteCameraOpposingXY( position, m_entityDef.GetVisualSize(), *g_game->GetWorldCamera(), corners );	 break;
		case eBillboardStyle::CAMERA_FACING_XYZ:	BillboardSpriteCameraFacingXYZ( position, m_entityDef.GetVisualSize(), *g_game->GetWorldCamera(), corners );	 break;
		case eBillboardStyle::CAMERA_OPPOSING_XYZ:	BillboardSpriteCameraOpposingXYZ( position, m_entityDef.GetVisualSize(), *g_game->GetWorldCamera(), corners ); break;
		
		default: BillboardSpriteCameraFacingXY( position, m_entityDef.GetVisualSize(), *g_game->GetWorldCamera(), corners ); break;
	}
	
	// Get UVs for sprite
	SpriteAnimDefinition* animDef = m_curSpriteAnimSetDef->GetSpriteAnimationDefForDirection( position.XY(), m_orientationDegrees, *g_game->GetWorldCamera() );
	const SpriteDefinition& spriteDef = animDef->GetSpriteDefAtTime( m_cumulativeTime );
	
	Vec2 mins, maxs;
	spriteDef.GetUVs( mins, maxs );

	std::vector<Vertex_PCUTBN> vertices;
	AppendVertsForQuad( vertices, corners, Rgba8::WHITE, mins, maxs );

	// Setup shaders and textures
	if ( m_curSpriteAnimSetDef->GetMaterial() != nullptr )
	{
		g_renderer->BindMaterial( m_curSpriteAnimSetDef->GetMaterial() );
	}
	else
	{
		// No material for this sprite, just use the diffuse texture
		g_renderer->BindShader( nullptr );
		g_renderer->BindDiffuseTexture( &(m_curSpriteAnimSetDef->GetTexture()) );
		g_renderer->BindNormalTexture( nullptr );
	}
	
	g_renderer->DrawVertexArray( vertices );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::Die()
{
	if ( IsDead() )
	{
		return;
	}

	m_isDead = true;

	
	/*	if ( IsScriptValid() )
		{
			EventArgs args;
			m_scriptObj->FireEvent( "OnDie", &args );
		}*/
	

}


//-----------------------------------------------------------------------------------------------
void GameEntity::DebugRender() const
{
	if ( m_isPossessed )
	{
		return;
	}

	Vec3 position = GetPosition();

	DebugAddWorldWireCylinder( position, Vec3( position.XY(), m_entityDef.m_height ), m_entityDef.m_physicsRadius, Rgba8::CYAN );
	//DebugAddWorldWireSphere( GetPosition() + m_collisionCenterOffset, m_entityDef.m_physicsRadius, Rgba8::YELLOW );
}


//-----------------------------------------------------------------------------------------------
const Vec2 GameEntity::GetForwardVector() const
{
	return Vec2::MakeFromPolarDegrees( m_orientationDegrees );
}


//-----------------------------------------------------------------------------------------------
const Vec2 GameEntity::GetRightVector() const
{
	return GetForwardVector().GetRotatedMinus90Degrees();
}


//-----------------------------------------------------------------------------------------------
const Vec3 GameEntity::GetUpVector() const
{
	return Transform::GetWorldUpVector();
}


//-----------------------------------------------------------------------------------------------
const Vec3 GameEntity::GetPosition() const
{
	if ( m_rigidbody == nullptr )
	{
		return Vec3::ZERO;
	}

	return m_rigidbody->GetWorldPosition();
}


//-----------------------------------------------------------------------------------------------
void GameEntity::SetPosition( const Vec3& position )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->SetPosition( position );
}


//-----------------------------------------------------------------------------------------------
const float GameEntity::GetMass() const
{
	if ( m_rigidbody == nullptr )
	{
		g_devConsole->PrintWarning( "No rigidbody for entity, returning default mass." );
		return 1.f;
	}

	return 1.f;
}


//-----------------------------------------------------------------------------------------------
void GameEntity::Translate( const Vec2& translation )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->AddImpulse( Vec3( translation, 0.f ) );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::Translate( const Vec3& translation )
{
	if ( m_rigidbody == nullptr )
	{
		return;
	}

	m_rigidbody->AddImpulse( translation );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::RotateDegrees( float pitchDegrees, float yawDegrees, float rollDegrees )
{
	UNUSED( pitchDegrees );
	UNUSED( rollDegrees );

	m_orientationDegrees += yawDegrees;
}


//-----------------------------------------------------------------------------------------------
//void Entity::MoveInCircle( const Vec3& center, float radius, float speed )
//{
//	m_orientationDegrees += speed;
//	if ( m_orientationDegrees > 360.f )
//	{
//		m_orientationDegrees -= 360.f;
//	}
//	if ( m_orientationDegrees < 0.f )
//	{
//		m_orientationDegrees += 360.f;
//	}
//	m_position = center + Vec3( Vec2::MakeFromPolarDegrees( m_orientationDegrees, radius ), 0.f );
//}


//-----------------------------------------------------------------------------------------------
void GameEntity::MoveInDirection( float speed, const Vec3& direction )
{
	AddForce( direction * speed );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::MoveInRelativeDirection( float speed, const Vec3& direction )
{
	Vec3 translationVector( direction.x * Vec3( GetForwardVector(), 0.f )
							+ direction.y * Vec3( GetRightVector(), 0.f )
							+ direction.z * GetUpVector() );

	AddForce( translationVector * speed );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::Possess()
{
	m_isPossessed = true;

	ZephyrSystem::FireScriptEvent( m_zephyrComponent, "OnPossess" );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::Unpossess()
{
	m_isPossessed = false;

	ZephyrSystem::FireScriptEvent( m_zephyrComponent, "OnUnPossess" );
}


//-----------------------------------------------------------------------------------------------
void GameEntity::ChangeSpriteAnimation( const std::string& spriteAnimDefSetName )
{
	SpriteAnimationSetDefinition* newSpriteAnimSetDef = m_entityDef.GetSpriteAnimSetDef( spriteAnimDefSetName );

	if ( newSpriteAnimSetDef == nullptr )
	{
		//g_devConsole->PrintWarning( Stringf( "Warning: Failed to change animation for entity '%s' to undefined animation '%s'", GetName().c_str(), spriteAnimDefSetName.c_str() ) );
		return;
	}

	m_curSpriteAnimSetDef = newSpriteAnimSetDef;
}


//-----------------------------------------------------------------------------------------------
void GameEntity::PlaySpriteAnimation( const std::string& spriteAnimDefSetName )
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
void GameEntity::TakeDamage( int damage )
{
	m_curHealth -= damage;
	if ( m_curHealth <= 0 )
	{
		Die();
	}
	
	g_game->AddScreenShakeIntensity(.05f);
}


//-----------------------------------------------------------------------------------------------
void GameEntity::AddForce( const Vec3& force )
{
	if ( m_rigidbody != nullptr )
	{
		m_rigidbody->AddForce( force );
	}
}


//-----------------------------------------------------------------------------------------------
void GameEntity::AddImpulse( const Vec3& impulse )
{
	if ( m_rigidbody != nullptr )
	{
		m_rigidbody->AddImpulse( impulse );
	}
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

	auto eventIter = m_registeredKeyEvents.find( keyCode );
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


////-----------------------------------------------------------------------------------------------
//ZephyrValue Entity::GetGlobalVariable( const std::string& varName )
//{
//	if ( !IsScriptValid() )
//	{
//		return ZephyrValue( ERROR_ZEPHYR_VAL );
//	}
//
//	// First check c++ built in vars
//	if ( varName == "id" ) { return ZephyrValue( (float)GetId() ); }
//	if ( varName == "name" ) { return ZephyrValue( GetName() ); }
//	if ( varName == "health" ) { return ZephyrValue( (float)m_curHealth ); }
//	if ( varName == "position" ) { return ZephyrValue( GetPosition() ); }
//	if ( varName == "forwardVec" ) { return ZephyrValue( GetForwardVector() ); }
//
//	return ZephyrEntity::GetGlobalVariable( varName );
//}
//
//
////-----------------------------------------------------------------------------------------------
//void Entity::SetGlobalVariable( const std::string& varName, const ZephyrValue& value )
//{
//	if ( !IsScriptValid() )
//	{
//		return;
//	}
//
//	// First check c++ built in vars
//	if ( varName == "health" )
//	{
//		m_curHealth = (int)value.GetAsNumber();
//		if ( m_curHealth < 0 )
//		{
//			Die();
//		}
//
//		return;
//	}
//
//	ZephyrEntity::SetGlobalVariable( varName, value );
//}


//-----------------------------------------------------------------------------------------------
void GameEntity::UpdateFromKeyboard( float deltaSeconds )
{
	UNUSED( deltaSeconds );

	if ( g_devConsole->IsOpen()
		 || m_isDead
		 /*|| g_game->GetGameState() != eGameState::PLAYING */)
	{
		return;
	}
	
	// Update script button events
	for ( auto& registeredKey : m_registeredKeyEvents )
	{
		if ( g_inputSystem->IsKeyPressed( registeredKey.first ) )
		{
			for ( auto& eventName : registeredKey.second )
			{
				EventArgs args;
				ZephyrSystem::FireScriptEvent( m_zephyrComponent, eventName, &args );
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
char GameEntity::GetKeyCodeFromString( const std::string& keyCodeStr )
{
	if (		IsEqualIgnoreCase( keyCodeStr, "space" ) )			{ return KEY_SPACEBAR; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "enter" ) )			{ return KEY_ENTER; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "shift" ) )			{ return KEY_SHIFT; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "left" ) )			{ return KEY_LEFTARROW; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "right" ) )			{ return KEY_RIGHTARROW; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "up" ) )				{ return KEY_UPARROW; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "down" ) )			{ return KEY_DOWNARROW; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "leftClick" ) )		{ return MOUSE_LBUTTON; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "rightClick" ) )		{ return MOUSE_RBUTTON; }
	else if (	IsEqualIgnoreCase( keyCodeStr, "middleClick" ) )	{ return MOUSE_MBUTTON; }
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

