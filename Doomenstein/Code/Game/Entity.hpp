#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/ZephyrCore/ZephyrCommon.hpp"
#include "Engine/ZephyrCore/ZephyrEntity.hpp"
#include "Game/EntityDefinition.hpp"

#include <string>
#include <vector>
#include <unordered_set>


//-----------------------------------------------------------------------------------------------
class Map;
class Rigidbody;
class SpriteAnimationSetDefinition;
class Texture;


//-----------------------------------------------------------------------------------------------
enum class eLightType
{
	STATIC_LIGHT,
	DYNAMIC_LIGHT,
};


//-----------------------------------------------------------------------------------------------
struct GameLight
{
	Light light;

	eLightType type = eLightType::DYNAMIC_LIGHT;
	bool isEnabled = false;
	int poolIndex = -1;
};


//-----------------------------------------------------------------------------------------------
class Entity : public ZephyrEntity
{
	friend class Map;
	friend class TileMap;
	friend class LineMap;
	
public:
	Entity( const EntityDefinition& entityDef, Map* map );
	virtual ~Entity() {}

	virtual void		Update( float deltaSeconds ) override;
	virtual void		Render() const;
	virtual void		Die() override;
	virtual void		DebugRender() const;

	// Accessors
	const Vec2			GetForwardVector() const;
	const Vec2			GetRightVector() const;
	const Vec3			GetUpVector() const;
	const Vec3			GetPosition() const;
	void				SetPosition( const Vec3& position );
	const float			GetPhysicsRadius() const								{ return m_entityDef.m_physicsRadius; }
	const float			GetHeight() const										{ return m_entityDef.m_height; }
	const float			GetEyeHeight() const									{ return m_entityDef.m_eyeHeight; }
	std::vector<ColliderData> GetColliderDataVec() const						{ return m_entityDef.m_colliderDataVec; }
	const float			GetMass() const;
	const float			GetOrientationDegrees() const							{ return m_orientationDegrees; }
	void				SetOrientationDegrees( float orientationDegrees )		{ m_orientationDegrees = orientationDegrees; }
	std::string			GetType() const											{ return m_entityDef.m_type; }
	eEntityClass		GetClass() const										{ return m_entityDef.m_class; }
	Map*				GetMap() const											{ return m_map; }
	void				SetMap( Map* map )										{ m_map = map; }
	void				SetRigidbody( Rigidbody* rigidbody )					{ m_rigidbody = rigidbody; }

	virtual bool		IsDead() const											{ return m_isDead; }
	bool				IsGarbage() const										{ return m_isGarbage; }
	bool				IsPossessed() const										{ return m_isPossessed; }

	// Physics
	void				Translate( const Vec2& translation );
	void				Translate( const Vec3& translation );
	void				RotateDegrees( float pitchDegrees, float yawDegrees, float rollDegrees );

	//void				MoveInCircle( const Vec3& center, float radius, float speed );
	void				MoveInDirection( float speed, const Vec3& direction );
	void				MoveInRelativeDirection( float speed, const Vec3& direction );

	// Pawn possession
	void				Possess();
	void				Unpossess();
						
	// Animation
	void				ChangeSpriteAnimation( const std::string& spriteAnimDefSetName );
	void				PlaySpriteAnimation( const std::string& spriteAnimDefSetName );
	SpriteAnimationSetDefinition* GetSpriteAnimSetDef( const std::string& animSetName ) const { return m_entityDef.GetSpriteAnimSetDef( animSetName ); }


	void				TakeDamage( int damage );
	void				AddForce( const Vec3& force );
	void				AddImpulse( const Vec3& impulse );

	// Input
	void				RegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName );
	void				UnRegisterKeyEvent( const std::string& keyCodeStr, const std::string& eventName );

	// Script overrides
	virtual ZephyrValue	GetGlobalVariable( const std::string& varName ) override;
	virtual void		SetGlobalVariable( const std::string& varName, const ZephyrValue& value ) override;
	virtual void		AddGameEventParams( EventArgs* args ) const override;

protected:
	void				UpdateFromKeyboard( float deltaSeconds );
	
	char				GetKeyCodeFromString( const std::string& keyCodeStr );

protected:
	// Game state
	const EntityDefinition&			m_entityDef;
	int								m_curHealth = 1;								// how much health is currently remaining on entity
	Map*							m_map = nullptr;

	bool							m_isDead = false;								// whether the Entity is ?dead? in the game; affects entity and game logic
	bool							m_isGarbage = false;							// whether the Entity should be deleted at the end of Game::Update()
	bool							m_isPossessed = false;							

	// Physics
	Rigidbody*						m_rigidbody = nullptr;
	float							m_orientationDegrees = 0.f;						// the Entity's forward - facing direction, as an angle in degrees

	std::unordered_set<EntityId>	m_collidingEntities;

	// Visual
	float							m_cumulativeTime = 0.f;
	std::vector<Vertex_PCU>			m_vertices;
	SpriteAnimationSetDefinition*	m_curSpriteAnimSetDef = nullptr;
	Texture*						m_texture = nullptr;
	GameLight						m_gameLight;

	// Input
	std::map<char, std::vector<std::string>> m_registeredKeyEvents;
};


//-----------------------------------------------------------------------------------------------
typedef std::vector<Entity*> EntityVector;
