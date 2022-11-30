#include "Game/EntityDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Zephyr/Core/ZephyrScriptDefinition.hpp"
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
		ParseZephyrCompDef( m_type, *scriptElem );
	}

	m_isValid = true;
}


//-----------------------------------------------------------------------------------------------
EntityDefinition::~EntityDefinition()
{
	PTR_MAP_SAFE_DELETE( m_spriteAnimSetDefs );
}


//-----------------------------------------------------------------------------------------------
void EntityDefinition::ParseZephyrCompDef( const std::string& entityType, const XmlElement& scriptElem )
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
	while ( globalVarElem != nullptr )
	{
		std::string typeName = ParseXmlAttribute( *globalVarElem, "type", "" );
		std::string varName = ParseXmlAttribute( *globalVarElem, "var", "" );
		std::string valueStr = ParseXmlAttribute( *globalVarElem, "value", "" );
		if ( typeName.empty() )
		{
			g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': GlobalVar is missing a variable type", entityType.c_str() ) );
			break;
		}
		if ( varName.empty() )
		{
			g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': GlobalVar is missing a variable name", entityType.c_str() ) );
			break;
		}
		if ( valueStr.empty() )
		{
			g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': GlobalVar is missing a variable value", entityType.c_str() ) );
			break;
		}

		if ( varName == PARENT_ENTITY_STR )
		{
			g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': GlobalVar cannot initialize reserved entity variable '%s'.", entityType.c_str(), PARENT_ENTITY_STR.c_str() ) );
			break;
		}

		// Convert value to correct type and store in map
		if ( !_strcmpi( typeName.c_str(), "string" ) )
		{
			m_zephyrDef->zephyrScriptInitialValues[varName] = ZephyrValue( valueStr );
		}
		else if ( !_strcmpi( typeName.c_str(), "number" ) )
		{
			m_zephyrDef->zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, 0.f ) );
		}
		else if ( !_strcmpi( typeName.c_str(), "bool" ) )
		{
			m_zephyrDef->zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, false ) );
		}
		else if ( !_strcmpi( typeName.c_str(), "vec2" ) )
		{
			m_zephyrDef->zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, Vec2::ZERO ) );
		}
		else if ( !_strcmpi( typeName.c_str(), "vec3" ) )
		{
			m_zephyrDef->zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, Vec3::ZERO ) );
		}
		else if ( !_strcmpi( typeName.c_str(), "entity" ) )
		{
			m_zephyrDef->zephyrEntityVarInits.emplace_back( varName, valueStr );
		}
		else
		{
			g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': GlobalVar '%s' has unsupported type '%s'", entityType.c_str(), varName.c_str(), typeName.c_str() ) );
			break;
		}

		globalVarElem = globalVarElem->NextSiblingElement( "GlobalVar" );
	}

	m_zephyrDef->isScriptValid = true;
}
