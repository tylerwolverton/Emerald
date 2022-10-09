#include "Engine/Zephyr/GameInterface/ZephyrComponentDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Zephyr/Core/ZephyrScriptDefinition.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrComponentDefinition::ZephyrComponentDefinition( const std::string& entityType, const XmlElement& scriptElem )
{	
	m_zephyrScriptName = ParseXmlAttribute( scriptElem, "name", "" );

	if ( m_zephyrScriptName.empty() )
	{
		g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': Script element has no name", entityType.c_str() ) );
		return;
	}

	m_zephyrScriptDef = ZephyrScriptDefinition::GetZephyrScriptDefinitionByName( m_zephyrScriptName );

	if ( m_zephyrScriptDef == nullptr )
	{
		g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': Script '%s' does not exist", entityType.c_str(), m_zephyrScriptName.c_str() ) );
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

		if ( varName == PARENT_ENTITY_NAME )
		{
			g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': GlobalVar cannot initialize reserved entity variable '%s'.", entityType.c_str(), PARENT_ENTITY_NAME.c_str() ) );
			break;
		}

		// Convert value to correct type and store in map
		if ( !_strcmpi( typeName.c_str(), "string" ) )
		{
			m_zephyrScriptInitialValues[varName] = ZephyrValue( valueStr );
		}
		else if ( !_strcmpi( typeName.c_str(), "number" ) )
		{
			m_zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, 0.f ) );
		}
		else if ( !_strcmpi( typeName.c_str(), "bool" ) )
		{
			m_zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, false ) );
		}
		else if ( !_strcmpi( typeName.c_str(), "vec2" ) )
		{
			m_zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, Vec2::ZERO ) );
		}
		else if ( !_strcmpi( typeName.c_str(), "vec3" ) )
		{
			m_zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, Vec3::ZERO ) );
		}
		else if ( !_strcmpi( typeName.c_str(), "entity" ) )
		{
			m_zephyrEntityVarInits.emplace_back( varName, valueStr );
		}
		else
		{
			g_devConsole->PrintError( Stringf( "EntityTypes.xml '%s': GlobalVar '%s' has unsupported type '%s'", entityType.c_str(), varName.c_str(), typeName.c_str() ) );
			break;
		}

		globalVarElem = globalVarElem->NextSiblingElement( "GlobalVar" );
	}
	
	m_isScriptValid = true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrComponentDefinition::ReloadZephyrScriptDefinition()
{
	if ( !m_zephyrScriptName.empty() )
	{
		m_zephyrScriptDef = ZephyrScriptDefinition::GetZephyrScriptDefinitionByName( m_zephyrScriptName );
	}
}
