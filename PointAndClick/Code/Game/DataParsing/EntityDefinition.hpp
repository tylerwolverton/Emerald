#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Game/Core/GameCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
struct ZephyrComponentDefinition;
class SpriteSheet;
class SpriteAnimationSetDefinition;


//-----------------------------------------------------------------------------------------------
class EntityDefinition
{
	friend class GameEntity;

public:
	explicit EntityDefinition( const XmlElement& entityDefElem, SpriteSheet* defaultSpriteSheet );
	virtual ~EntityDefinition();
	
	bool			IsValid() const																{ return m_isValid; }
	std::string		GetType() const																{ return m_type; }
	float			GetMaxHealth() const														{ return m_maxHealth; }

	bool						HasZephyrScript() const											{ return m_zephyrDef != nullptr; }
	ZephyrComponentDefinition*	GetZephyrCompDef() const										{ return m_zephyrDef; }

	SpriteAnimationSetDefinition* GetDefaultSpriteAnimSetDef() const							{ return m_defaultSpriteAnimSetDef; }
	std::map< std::string, SpriteAnimationSetDefinition* > GetSpriteAnimSetDefs() const			{ return m_spriteAnimSetDefs; }
	SpriteAnimationSetDefinition* GetSpriteAnimSetDef( const std::string& animSetName ) const;
	
	static EntityDefinition* GetEntityDefinition( std::string entityName );

public:
	static std::map< std::string, EntityDefinition* > s_definitions;

private:
	void ParseZephyrCompDef( const std::string& entityType, const XmlElement& scriptElem );

protected:
	bool			m_isValid = false;
	std::string		m_type;
	float			m_maxHealth = 1.f;

	ZephyrComponentDefinition* m_zephyrDef = nullptr;

	AABB2			m_localDrawBounds;
	AABB2			m_uvCoords = AABB2::ONE_BY_ONE;
	
	std::map< std::string, SpriteAnimationSetDefinition* > m_spriteAnimSetDefs;
	SpriteAnimationSetDefinition* m_defaultSpriteAnimSetDef = nullptr;
};
