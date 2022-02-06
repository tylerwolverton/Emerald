#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/ZephyrCore/ZephyrCommon.hpp"
#include "Engine/ZephyrCore/ZephyrEntityDefinition.hpp"
#include "Game/GameCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class Material;
class SpriteSheet;
class SpriteAnimationSetDefinition;


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
class EntityDefinition : public ZephyrEntityDefinition
{
	friend class Entity;

public:
	explicit EntityDefinition( const XmlElement& entityDefElem );
	~EntityDefinition();

	bool			IsValid() const																{ return m_isValid; }
	std::string		GetName() const																{ return m_type; }
	eEntityClass	GetClass() const															{ return m_class; }
	float			GetWalkSpeed() const														{ return m_walkSpeed; }
	Vec2			GetVisualSize() const														{ return m_visualSize; }

	SpriteAnimationSetDefinition* GetDefaultSpriteAnimSetDef() const							{ return m_defaultSpriteAnimSetDef; }
	std::map< std::string, SpriteAnimationSetDefinition* > GetSpriteAnimSetDefs() const			{ return m_spriteAnimSetDefs; }
	SpriteAnimationSetDefinition* GetSpriteAnimSetDef( const std::string& animSetName ) const;

	static EntityDefinition* GetEntityDefinition( std::string entityName );

public:
	static std::map< std::string, EntityDefinition* > s_definitions;

protected:
	bool			m_isValid = false;
	std::string		m_type;
	eEntityClass	m_class = eEntityClass::UNKNOWN;

	float			m_physicsRadius = 0.f;
	float			m_height = 0.f;
	float			m_eyeHeight = 0.f;
	float			m_mass = 1.f;
	float			m_walkSpeed = 0.f;

	std::string		m_initialCollisionLayer;
	bool			m_initialCanBePushed = false;
	bool			m_initialCanPush = false;

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