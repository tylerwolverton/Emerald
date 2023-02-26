#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Framework/Entity.hpp"
#include "Engine/Time/Timer.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include "Game/DataParsing/EntityDefinition.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Map;
class Collision;
class Rigidbody;
class SpriteAnimDefinition;
class ZephyrScript;


//-----------------------------------------------------------------------------------------------
class GameEntity : public Entity
{
	friend class Map;
	
public:
	GameEntity( const EntityDefinition& entityDef, Map* map );
	virtual ~GameEntity();

	void				Update( float deltaSeconds );
	void				Render() const;
	void				Die();
	void				DebugRender() const;

	void				Load();
	void				Unload();

	void				UpdateFromKeyboard(float deltaSeconds);

	void				ChangeSpriteAnimation( const std::string& spriteAnimDefSetName );
	void				PlaySpriteAnimation( const std::string& spriteAnimDefSetName );
	SpriteAnimationSetDefinition* GetSpriteAnimSetDef( const std::string& animSetName ) const { return m_entityDef.GetSpriteAnimSetDef( animSetName ); }


	const Vec2			GetForwardVector() const;
	const Vec3			GetPosition() const;
	void				SetPosition( const Vec3& position );
	const float			GetOrientationDegrees() const							{ return m_transform.GetYawDegrees(); }
	void				SetOrientationDegrees( float orientationDegrees );

	std::string			GetType() const											{ return m_entityDef.m_type; }
	Map*				GetMap() const											{ return m_map; }
	void				SetMap( Map* map )										{ m_map = map; }
			
	// TODO: See if there's a better way to do this
	void				SetAsPlayer();

	bool				IsDead() const 											{ return m_isDead; }
	bool				IsGarbage() const										{ return m_isGarbage; }
	bool				IsPlayer() const										{ return m_isPlayer; }
				 
	void				RegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName );
	void				UnRegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName );

	// Script wrappers
	void				FireSpawnEvent();
	bool				FireScriptEvent( const std::string& eventName, EventArgs* args = nullptr );

	ZephyrValue			GetGlobalVariable( const std::string& varName );
	void				SetGlobalVariable( const std::string& varName, const ZephyrValue& value );

protected:
	char				GetKeyCodeFromString( const std::string& keyCodeStr );

protected:
	// Game state
	const EntityDefinition&					m_entityDef;
	Transform								m_transform;
	float									m_curHealth = 1.f;								// how much health is currently remaining on entity
	Map*									m_map = nullptr;
	
	bool									m_isDead = false;								// whether the Entity is “dead” in the game; affects entity and game logic
	bool									m_isGarbage = false;							// whether the Entity should be deleted at the end of Game::Update()
	bool									m_isPlayer = false;

	GameEntity*								m_dialoguePartner = nullptr;
		
	// Visual
	float									m_cumulativeTime = 0.f;
	Vec2									m_facingDirection = Vec2::ZERO_TO_ONE;
	std::vector<Vertex_PCU>					m_vertices;
	SpriteAnimationSetDefinition*			m_curSpriteAnimSetDef = nullptr;

	// Input
	std::map<char, std::vector<std::string>> m_registeredKeyEvents;
};


//-----------------------------------------------------------------------------------------------
typedef std::vector<GameEntity*> EntityVector;
