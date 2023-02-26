#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Game/Core/GameCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
struct SpriteAnimationComponentDefinition;
struct ZephyrComponentDefinition;
class SpriteSheet;
class SpriteAnimationSetDefinition;


//-----------------------------------------------------------------------------------------------
class EntityTypeDefinition
{
	friend class GameEntity;

public:
	explicit EntityTypeDefinition( const XmlElement& entityDefElem, SpriteSheet* defaultSpriteSheet );
	virtual ~EntityTypeDefinition();
	
	bool			IsValid() const																{ return m_isValid; }
	std::string		GetType() const																{ return m_type; }
	float			GetMaxHealth() const														{ return m_maxHealth; }

	bool								HasZephyrScript() const									{ return m_zephyrDef != nullptr; }
	ZephyrComponentDefinition*			GetZephyrCompDef() const								{ return m_zephyrDef; }	
	bool								HasSpriteAnimation() const								{ return m_spriteAnimCompDef != nullptr; }
	SpriteAnimationComponentDefinition*	GetSpriteAnimationCompDef() const						{ return m_spriteAnimCompDef; }
	
	static EntityTypeDefinition* GetEntityDefinition( std::string entityName );

public:
	static std::map< std::string, EntityTypeDefinition* > s_definitions;

private:
	void ParseZephyrCompDef( const std::string& entityType, const XmlElement& scriptElem );
	void ParseSpriteAnimCompDef( SpriteSheet* spriteSheet, const XmlElement& spriteAnimationElem );
	SpriteSheet* ParseSpriteSheet( const XmlElement& spriteSheetElem );

protected:
	bool			m_isValid = false;
	std::string		m_type;
	float			m_maxHealth = 1.f;

	ZephyrComponentDefinition*			m_zephyrDef = nullptr;
	SpriteAnimationComponentDefinition* m_spriteAnimCompDef = nullptr;
};
