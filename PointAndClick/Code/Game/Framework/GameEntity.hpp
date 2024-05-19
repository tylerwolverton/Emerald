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

#include "Game/DataParsing/EntityTypeDefinition.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Map;
class Collision;
class Rigidbody;
class SpriteAnimDefinition;
class ZephyrScript;


//-----------------------------------------------------------------------------------------------
// GameEntity provides helpful wrappers to cpp code for manipulating entities
//-----------------------------------------------------------------------------------------------
class GameEntity : public Entity
{
	friend class Map;
	
public:
	GameEntity( const EntityTypeDefinition& entityDef, Map* map );
	virtual ~GameEntity();

	void				DebugRender() const;

	void				Load();
	void				Unload();

	void				Die();

	void				UpdateFromKeyboard();

	void				ChangeSpriteAnimation( const std::string& spriteAnimDefSetName );
	void				PlaySpriteAnimation( const std::string& spriteAnimDefSetName );

	const Vec2			GetForwardVector() const;
	const Vec3			GetPosition() const;
	void				SetPosition( const Vec3& position );
	const float			GetOrientationDegrees() const							{ return m_transform.GetYawDegrees(); }
	void				SetOrientationDegrees( float orientationDegrees );
	float				GetInteractionRadius()									{ return m_entityDef.m_interactionRadius; }

	std::string			GetType() const											{ return m_entityDef.m_type; }
	Map*				GetMap() const											{ return m_map; }
	void				SetMap( Map* map )										{ m_map = map; }

	bool				IsDead() const 											{ return m_isDead; }
	bool				IsGarbage() const										{ return m_isGarbage; }
				 
	void				RegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName );
	void				UnRegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName );

	// Script wrappers
	void				FireSpawnEvent();
	bool				FireScriptEvent( const std::string& eventName, EventArgs* args = nullptr );

	ZephyrHandle		GetGlobalVariable( const std::string& varName );
	bool				SetGlobalVariable( const std::string& varName, const ZephyrValue& value );

protected:
	char				GetKeyCodeFromString( const std::string& keyCodeStr );

protected:
	// Game state
	const EntityTypeDefinition&				m_entityDef;
	Map*									m_map = nullptr;
	Transform								m_transform;
	float									m_curHealth = 1.f;								
	
	bool									m_isDead = false;								// whether the Entity is “dead” in the game; affects entity and game logic
	bool									m_isGarbage = false;							// whether the Entity should be deleted at the end of Game::Update()

	GameEntity*								m_dialoguePartner = nullptr;
	
	// Visual
	Vec2									m_facingDirection = Vec2::ZERO_TO_ONE;

	// Input
	std::map<char, std::vector<std::string>> m_registeredKeyEvents;
};


//-----------------------------------------------------------------------------------------------
typedef std::vector<GameEntity*> EntityVector;
