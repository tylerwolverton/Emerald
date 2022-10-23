#include "Game/EntityDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponentDefinition.hpp"

#include "Game/SpriteAnimationSetDefinition.hpp"


//-----------------------------------------------------------------------------------------------
std::map< std::string, EntityDefinition* > EntityDefinition::s_definitions;


//-----------------------------------------------------------------------------------------------
SpriteAnimationSetDefinition* EntityDefinition::GetSpriteAnimSetDef( const std::string& animSetName ) const
{
	auto mapIter = m_spriteAnimSetDefs.find( animSetName );

	if ( mapIter == m_spriteAnimSetDefs.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
EntityDefinition* EntityDefinition::GetEntityDefinition( std::string entityName )
{
	std::map< std::string, EntityDefinition* >::const_iterator  mapIter = EntityDefinition::s_definitions.find( entityName );

	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
EntityDefinition::EntityDefinition( const XmlElement& entityDefElem, SpriteSheet* defaultSpriteSheet )
{
	m_type = ParseXmlAttribute( entityDefElem, "name", "" );
	if ( m_type == "" )
	{
		g_devConsole->PrintError( "EntityTypes.xml: EntityType is missing a name attribute" );
		return;
	}

	// Appearance
	const XmlElement* appearanceElem = entityDefElem.FirstChildElement( "Appearance" );
	if ( appearanceElem != nullptr )
	{
		SpriteSheet* spriteSheet = defaultSpriteSheet;

		std::string spriteSheetPath = ParseXmlAttribute( *appearanceElem, "spriteSheet", "" );
		if ( spriteSheetPath != "" )
		{
			spriteSheet = SpriteSheet::GetSpriteSheetByPath( spriteSheetPath );
			if ( spriteSheet == nullptr )
			{
				IntVec2 spriteSheetDimensions = ParseXmlAttribute( *appearanceElem, "spriteSheetDimensions", IntVec2( -1, -1 ) );
				if ( spriteSheetDimensions == IntVec2( -1, -1 ) )
				{
					g_devConsole->PrintError( Stringf( "EntityTypes.xml: '%s' Missing layout attribute", m_type.c_str() ) );
					return;
				}

				Texture* texture = g_renderer->CreateOrGetTextureFromFile( spriteSheetPath.c_str() );
				if ( texture == nullptr )
				{
					g_devConsole->PrintError( Stringf( "EntityTypes.xml: '%s' Couldn't load texture '%s'", spriteSheetPath.c_str(), m_type.c_str() ) );
					return;
				}

				spriteSheet = SpriteSheet::CreateAndRegister( *texture, spriteSheetDimensions );
			}
		}

		m_localDrawBounds = ParseXmlAttribute( *appearanceElem, "localDrawBounds", m_localDrawBounds );
		float defaultFPS = ParseXmlAttribute( *appearanceElem, "fps", 1.f );

		bool isFirstAnim = true;

		const XmlElement* animationSetElem = appearanceElem->FirstChildElement();
		while ( animationSetElem != nullptr )
		{
			m_spriteAnimSetDefs[animationSetElem->Name()] = new SpriteAnimationSetDefinition( spriteSheet, *animationSetElem, defaultFPS );

			if ( isFirstAnim )
			{
				isFirstAnim = false;
				m_defaultSpriteAnimSetDef = m_spriteAnimSetDefs[animationSetElem->Name()];
			}

			animationSetElem = animationSetElem->NextSiblingElement();
		}
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
		m_zephyrDef = new ZephyrComponentDefinition( m_type, *scriptElem );
	}

	m_isValid = true;
}


//-----------------------------------------------------------------------------------------------
EntityDefinition::~EntityDefinition()
{
	PTR_MAP_SAFE_DELETE( m_spriteAnimSetDefs );
}
