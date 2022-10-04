#pragma once
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Game/GameCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class Material;
class SpriteSheet;
class SpriteAnimationSetDefinition;
class ZephyrComponentDefinition;


//-----------------------------------------------------------------------------------------------
enum eEntityClass
{
	UNKNOWN = -1,
	ENTITY,
	ACTOR,
	PROJECTILE,
	PORTAL,
};

std::string GetEntityClassAsString( eEntityClass entityType );


//-----------------------------------------------------------------------------------------------
struct ColliderData
{
public:
	Vec3 offsetFromCenter = Vec3::ZERO;
	eColliderType type = COLLIDER_NONE;

	float radius = 1.f;
	OBB3 obb3;
	
};


//-----------------------------------------------------------------------------------------------
class EntityDefinition
{
	friend class GameEntity;

public:
	explicit EntityDefinition( const XmlElement& entityDefElem );
	~EntityDefinition();

	bool						IsValid() const																{ return m_isValid; }
	std::string					GetName() const																{ return m_type; }
	eEntityClass				GetClass() const															{ return m_class; }
	Vec2						GetVisualSize() const														{ return m_visualSize; }
	float						GetMass() const																{ return m_mass; }
	std::string					GetInitialCollisionLayer() const											{ return m_initialCollisionLayer; }
	bool						HasPhysics() const															{ return m_hasPhysics; }
	bool						HasZephyrScript() const														{ return m_zephyrDef != nullptr; }
	ZephyrComponentDefinition*	GetZephyrCompDef() const													{ return m_zephyrDef; }

	SpriteAnimationSetDefinition* GetDefaultSpriteAnimSetDef() const										{ return m_defaultSpriteAnimSetDef; }
	std::map< std::string, SpriteAnimationSetDefinition* > GetSpriteAnimSetDefs() const						{ return m_spriteAnimSetDefs; }
	SpriteAnimationSetDefinition* GetSpriteAnimSetDef( const std::string& animSetName ) const;

	static EntityDefinition* GetEntityDefinition( std::string entityName );

public:
	static std::map< std::string, EntityDefinition* > s_definitions;

protected:
	bool			m_isValid = false;
	std::string		m_type;
	eEntityClass	m_class = eEntityClass::UNKNOWN;

	// TODO: Move to vector of ComponentDefs? Don't really want EntityDef in engine though
	ZephyrComponentDefinition* m_zephyrDef = nullptr;


	bool			m_hasPhysics = false; // TODO: Go full ECS with this
	float			m_physicsRadius = 0.f;
	float			m_height = 0.f;
	float			m_eyeHeight = 0.f;
	float			m_mass = 1.f;

	std::string		m_initialCollisionLayer;
	std::vector<ColliderData> m_colliderDataVec;
	NamedProperties* m_colliderParams = nullptr; // TODO: Account for multiple colliders later if needed

	Vec2			m_visualSize = Vec2::ZERO;
	eBillboardStyle m_billboardStyle = eBillboardStyle::CAMERA_FACING_INVALID;
	Material*		m_spriteMaterial = nullptr;

	float			m_lightIntensity = 0.f;
	Vec3			m_lightColor = Vec3::ONE;
	Vec3			m_lightAttenuation = Vec3( 0.f, 1.f, 0.f );
	float			m_lightHalfCosOfInnerAngle = -1.f;
	Vec3			m_lightSpecularAttenuation = Vec3( 0.f, 1.f, 0.f );
	float			m_lightHalfCosOfOuterAngle = -1.f;
	bool			m_isLightEnabled = false;

	std::map< std::string, SpriteAnimationSetDefinition* > m_spriteAnimSetDefs;
	SpriteAnimationSetDefinition* m_defaultSpriteAnimSetDef = nullptr;
};
