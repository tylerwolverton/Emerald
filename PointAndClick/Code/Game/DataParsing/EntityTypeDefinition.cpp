#include "Game/DataParsing/EntityTypeDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Zephyr/Core/ZephyrScriptDefinition.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponentDefinition.hpp"

#include "Game/Core/GameCommon.hpp"
#include "Game/DataParsing/DataParsingUtils.hpp"
#include "Game/Graphics/SpriteAnimationSetDefinition.hpp"
#include "Game/Graphics/SpriteAnimationComponentDefinition.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, EntityTypeDefinition* > EntityTypeDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
EntityTypeDefinition* EntityTypeDefinition::GetEntityDefinition( std::string entityName )
{
	std::map< std::string, EntityTypeDefinition* >::const_iterator  mapIter = EntityTypeDefinition::s_definitions.find( entityName );

	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
EntityTypeDefinition::EntityTypeDefinition( const XmlElement& entityDefElem, SpriteSheet* defaultSpriteSheet )
{
	m_type = ParseXmlAttribute( entityDefElem, "name", "" );
	if ( m_type == "" )
	{
		g_devConsole->PrintError( "EntityTypes.xml: EntityType is missing a name attribute" );
		return;
	}

	// Physics
	const XmlElement* physicsElem = entityDefElem.FirstChildElement( "Physics" );
	if ( physicsElem != nullptr )
	{
		m_interactionRadius = ParseXmlAttribute( *physicsElem, "interactionRadius", 0.f );

	}

	// Appearance
	const XmlElement* appearanceElem = entityDefElem.FirstChildElement( "Appearance" );
	if ( appearanceElem != nullptr )
	{
		SpriteSheet* spriteSheet = ParseSpriteSheet( *appearanceElem );
		if ( spriteSheet == nullptr )
		{
			spriteSheet = defaultSpriteSheet;
		}

		ParseSpriteAnimCompDef( spriteSheet, *appearanceElem );
	}

	// Gameplay
	const XmlElement* gameplayElem = entityDefElem.FirstChildElement( "Gameplay" );
	if ( gameplayElem != nullptr )
	{
		m_maxHealth = ParseXmlAttribute( *gameplayElem, "maxHealth", m_maxHealth );
	}
	
	// Zephyr
	const XmlElement* scriptElem = entityDefElem.FirstChildElement( "Script" );
	if ( scriptElem != nullptr )
	{
		ParseZephyrCompDef( m_type, *scriptElem );
	}

	m_isValid = true;
}


//-----------------------------------------------------------------------------------------------
EntityTypeDefinition::~EntityTypeDefinition()
{
	PTR_SAFE_DELETE( m_zephyrDef );
	PTR_SAFE_DELETE( m_spriteAnimCompDef );
}


//-----------------------------------------------------------------------------------------------
void EntityTypeDefinition::ParseZephyrCompDef( const std::string& entityType, const XmlElement& scriptElem )
{
	m_zephyrDef = new ZephyrComponentDefinition();

	m_zephyrDef->zephyrScriptName = ParseXmlAttribute( scriptElem, "name", "" );

	if ( m_zephyrDef->zephyrScriptName.empty() )
	{
		g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': Script element has no name", entityType.c_str() ) );
		return;
	}

	m_zephyrDef->zephyrScriptDef = ZephyrScriptDefinition::GetZephyrScriptDefinitionByName( m_zephyrDef->zephyrScriptName );

	if ( m_zephyrDef->zephyrScriptDef == nullptr )
	{
		g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': Script '%s' does not exist", entityType.c_str(), m_zephyrDef->zephyrScriptName.c_str() ) );
		return;
	}

	// Parse initial values
	const XmlElement* globalVarElem = scriptElem.FirstChildElement( "GlobalVar" );
	const std::string errorStr = ParseZephyrVarInitsFromData( globalVarElem, m_zephyrDef->zephyrScriptInitialValues, m_zephyrDef->zephyrEntityVarInits );

	if ( !errorStr.empty() )
	{
		g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': %s", entityType.c_str(), errorStr.c_str() ) );
		return;
	}

	m_zephyrDef->isScriptValid = true;
}


//-----------------------------------------------------------------------------------------------
void EntityTypeDefinition::ParseSpriteAnimCompDef( SpriteSheet* spriteSheet, const XmlElement& spriteAnimationElem )
{
	m_spriteAnimCompDef = new SpriteAnimationComponentDefinition();

	const float UNITS_PER_PIXEL = 1.f / g_gameConfigBlackboard.GetValue( "pixelsPerUnit", DEFAULT_PIXELS_PER_UNIT );

	Vec2 localMaxs( (float)spriteSheet->GetSpriteDimensionsInPixels().x * UNITS_PER_PIXEL, (float)spriteSheet->GetSpriteDimensionsInPixels().y * UNITS_PER_PIXEL );
	AABB2 localDrawBounds( Vec2::ZERO, localMaxs );

	float spriteScale = ParseXmlAttribute( spriteAnimationElem, "spriteScale", 1.f );
	localDrawBounds.maxs *= spriteScale;

	IntVec2 localDrawOffsetInPixels = ParseXmlAttribute( spriteAnimationElem, "localDrawOffsetInPixels", IntVec2::ZERO );
	localDrawBounds.Translate( Vec2( (float)localDrawOffsetInPixels.x * UNITS_PER_PIXEL, (float)localDrawOffsetInPixels.y * UNITS_PER_PIXEL ) );

	m_spriteAnimCompDef->localDrawBounds = localDrawBounds;

	float defaultFPS = ParseXmlAttribute( spriteAnimationElem, "fps", 1.f );

	bool isFirstAnim = true;

	const XmlElement* animationSetElem = spriteAnimationElem.FirstChildElement();
	while ( animationSetElem != nullptr )
	{
		m_spriteAnimCompDef->spriteAnimSetDefs[animationSetElem->Name()] = new SpriteAnimationSetDefinition( spriteSheet, *animationSetElem, defaultFPS );

		if ( isFirstAnim )
		{
			isFirstAnim = false;
			m_spriteAnimCompDef->defaultSpriteAnimSetDef = m_spriteAnimCompDef->spriteAnimSetDefs[animationSetElem->Name()];
		}

		animationSetElem = animationSetElem->NextSiblingElement();
	}
}


//-----------------------------------------------------------------------------------------------
SpriteSheet* EntityTypeDefinition::ParseSpriteSheet( const XmlElement& spriteSheetElem )
{
	SpriteSheet* spriteSheet = nullptr;

	// Register a new sprite sheet if specified
	std::string spriteSheetPath = ParseXmlAttribute( spriteSheetElem, "spriteSheet", "" );
	if ( spriteSheetPath != "" )
	{
		spriteSheet = SpriteSheet::GetSpriteSheetByPath( spriteSheetPath );
		if ( spriteSheet == nullptr )
		{
			IntVec2 spriteSheetDimensions = ParseXmlAttribute( spriteSheetElem, "spriteSheetDimensions", IntVec2( -1, -1 ) );
			if ( spriteSheetDimensions == IntVec2( -1, -1 ) )
			{
				g_devConsole->PrintError( Stringf( "EntityTypes.xml: '%s' Missing layout attribute", m_type.c_str() ) );
				return nullptr;
			}

			Texture* texture = g_renderer->CreateOrGetTextureFromFile( spriteSheetPath.c_str() );
			if ( texture == nullptr )
			{
				g_devConsole->PrintError( Stringf( "EntityTypes.xml: '%s' Couldn't load texture '%s'", spriteSheetPath.c_str(), m_type.c_str() ) );
				return nullptr;
			}

			spriteSheet = SpriteSheet::CreateAndRegister( *texture, spriteSheetDimensions );
		}
	}

	return spriteSheet;
}
