#include "Game/MapDefinition.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"

#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
MapDefinition::MapDefinition( const XmlElement& mapDefElem, const std::string& mapName )
	: mapName( mapName )
{
	if ( !ParseMapDefinitionNode( mapDefElem ) ) { return; }
	if ( !ParseEntitiesNode( mapDefElem ) ) { return; }

	isValid = true;
}


//-----------------------------------------------------------------------------------------------
MapDefinition::~MapDefinition()
{	
}


//-----------------------------------------------------------------------------------------------
bool MapDefinition::ParseMapDefinitionNode( const XmlElement& mapDefElem )
{
	type = ParseXmlAttribute( mapDefElem, "type", type );
	if ( type == "InvalidType" )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing a type attribute", mapName.c_str() ) );
		return false;
	}

	version = ParseXmlAttribute( mapDefElem, "version", version );
	if ( version == -1 )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing a version attribute", mapName.c_str() ) );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool MapDefinition::ParseEntitiesNode( const XmlElement& mapDefElem )
{
	const XmlElement* entitiesElem = mapDefElem.FirstChildElement( "Entities" );
	if ( entitiesElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing an entities node", mapName.c_str() ) );
		return false;
	}

	bool hasParsedPlayerStart = false;
	const XmlElement* entityElem = entitiesElem->FirstChildElement();
	while ( entityElem )
	{
		if ( !strcmp( entityElem->Value(), "PlayerStart" ) )
		{
			if ( hasParsedPlayerStart )
			{
				g_devConsole->PrintError( Stringf( "Map file '%s' defines multiple player start nodes in Entities, the first one will be used", mapName.c_str() ) );
				continue;
			}

			playerStartPos = ParseXmlAttribute( *entityElem, "pos", playerStartPos );
			playerStartYaw = ParseXmlAttribute( *entityElem, "yaw", playerStartYaw );

			hasParsedPlayerStart = true;
		}
		else if ( !strcmp( entityElem->Value(), "Entity" ) )
		{
			CreateMapEntityDefFromNode( *entityElem, entityElem->Value() );
		}
		else
		{
			g_devConsole->PrintError( Stringf( "Entity type '%s' is unknown", entityElem->Value() ) );
		}

		entityElem = entityElem->NextSiblingElement();
	}

	if ( hasParsedPlayerStart == false )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing a player start node in Entities", mapName.c_str() ) );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
void MapDefinition::CreateMapEntityDefFromNode( const XmlElement& entityElem, const std::string& expectedType )
{
	MapEntityDefinition mapEntityDef;

	std::string entityType = ParseXmlAttribute( entityElem, "type", "" );
	mapEntityDef.name = ParseXmlAttribute( entityElem, "name", "" );
	mapEntityDef.entityDef = EntityDefinition::GetEntityDefinition( entityType );
	if ( mapEntityDef.entityDef == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s': Entity '%s' was not defined in EntityTypes.xml", mapName.c_str(), entityType.c_str() ) );
		return;
	}

	mapEntityDef.position = ParseXmlAttribute( entityElem, "pos", Vec2::ZERO );
	mapEntityDef.yawDegrees = ParseXmlAttribute( entityElem, "yaw", 0.f );

	const XmlElement* globalVarElem = entityElem.FirstChildElement( "GlobalVar" );
	while ( globalVarElem != nullptr )
	{
		std::string typeName = ParseXmlAttribute( *globalVarElem, "type", "" );
		std::string varName = ParseXmlAttribute( *globalVarElem, "var", "" );
		std::string valueStr = ParseXmlAttribute( *globalVarElem, "value", "" );
		if ( typeName.empty() )
		{
			g_devConsole->PrintError( Stringf( "Map file '%s': GlobalVar is missing a variable type", mapName.c_str() ) );
			break;
		}
		if ( varName.empty() )
		{
			g_devConsole->PrintError( Stringf( "Map file '%s': GlobalVar is missing a variable name", mapName.c_str() ) );
			break;
		}
		if ( valueStr.empty() )
		{
			g_devConsole->PrintError( Stringf( "Map file '%s': GlobalVar is missing a variable value", mapName.c_str() ) );
			break;
		}

		if ( varName == PARENT_ENTITY_NAME )
		{
			g_devConsole->PrintError( Stringf( "Map file '%s': GlobalVar cannot initialize reserved entity variable '%s'.", mapName.c_str(), PARENT_ENTITY_NAME.c_str() ) );
			break;
		}

		// Convert value to correct type and store in map
		if ( !_strcmpi( typeName.c_str(), "string" ) )
		{
			mapEntityDef.zephyrScriptInitialValues[varName] = ZephyrValue( valueStr );
		}
		else if ( !_strcmpi( typeName.c_str(), "number" ) )
		{
			mapEntityDef.zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, 0.f ) );
		}
		else if ( !_strcmpi( typeName.c_str(), "bool" ) )
		{
			mapEntityDef.zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, false ) );
		}
		else if ( !_strcmpi( typeName.c_str(), "vec2" ) )
		{
			mapEntityDef.zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, Vec2::ZERO ) );
		}
		else if ( !_strcmpi( typeName.c_str(), "vec3" ) )
		{
			mapEntityDef.zephyrScriptInitialValues[varName] = ZephyrValue( FromString( valueStr, Vec3::ZERO ) );
		}
		else if ( !_strcmpi( typeName.c_str(), "entity" ) )
		{
			mapEntityDef.zephyrEntityVarInits.emplace_back( varName, valueStr );
		}
		else
		{
			g_devConsole->PrintError( Stringf( "Map file '%s': GlobalVar '%s' has unsupported type '%s'", mapName.c_str(), varName.c_str(), typeName.c_str() ) );
			break;
		}

		globalVarElem = globalVarElem->NextSiblingElement( "GlobalVar" );
	}

	mapEntityDefs.push_back( mapEntityDef );
}

