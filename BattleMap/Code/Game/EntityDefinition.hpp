#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/ZephyrCore/ZephyrCommon.hpp"
#include "Engine/ZephyrCore/ZephyrEntityDefinition.hpp"
#include "Game/GameCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class SpriteSheet;
class SpriteAnimationSetDefinition;
class ZephyrScriptDefinition;


//-----------------------------------------------------------------------------------------------
class EntityDefinition : public ZephyrEntityDefinition
{
	friend class Entity;

public:
	explicit EntityDefinition( const XmlElement& entityDefElem, SpriteSheet* spriteSheet );
	virtual ~EntityDefinition();
	
	bool			IsValid() const																{ return m_isValid; }
	std::string		GetType() const																{ return m_type; }
	float			GetMaxHealth() const														{ return m_maxHealth; }

	FloatRange		GetDamageRange() const														{ return m_damageRange; }

	SpriteAnimationSetDefinition* GetDefaultSpriteAnimSetDef() const							{ return m_defaultSpriteAnimSetDef; }
	std::map< std::string, SpriteAnimationSetDefinition* > GetSpriteAnimSetDefs() const			{ return m_spriteAnimSetDefs; }
	SpriteAnimationSetDefinition* GetSpriteAnimSetDef( const std::string& animSetName ) const;
	
	static EntityDefinition* GetEntityDefinition( std::string entityName );

public:
	static std::map< std::string, EntityDefinition* > s_definitions;

protected:
	bool			m_isValid = false;
	std::string		m_type;
	float			m_maxHealth = 1.f;

	FloatRange		m_damageRange = FloatRange( 0.f );

	AABB2			m_localDrawBounds;
	AABB2			m_uvCoords = AABB2::ONE_BY_ONE;
	
	std::map< std::string, SpriteAnimationSetDefinition* > m_spriteAnimSetDefs;
	SpriteAnimationSetDefinition* m_defaultSpriteAnimSetDef = nullptr;
};
