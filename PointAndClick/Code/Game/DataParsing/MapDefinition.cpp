#include "Game/DataParsing/MapDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"

#include "Game/DataParsing/DataParsingUtils.hpp"
#include "Game/DataParsing/EntityTypeDefinition.hpp"
#include "Game/Framework/Map.hpp"


//-----------------------------------------------------------------------------------------------
MapDefinition::MapDefinition( const std::string& mapFullPath )
{
	mapName = GetFileNameWithoutExtension( mapFullPath );

	std::string fileExt = GetFileExtension( mapFullPath );
	if ( IsEqualIgnoreCase(fileExt, ".xml") )
	{
		XmlDocument doc;
		XmlError loadError = doc.LoadFile( mapFullPath.c_str() );
		if ( loadError != tinyxml2::XML_SUCCESS )
		{
			g_devConsole->PrintError( Stringf( "'%s' could not be opened", mapFullPath.c_str() ) );
			return;
		}

		XmlElement* root = doc.RootElement();
		if ( strcmp( root->Name(), "MapDefinition" ) )
		{
			g_devConsole->PrintError( Stringf( "'%s': Incorrect root node name, must be MapDefinition", mapFullPath.c_str() ) );
			return;
		}

		if ( !ParseMapDefinitionXmlNode( *root ) ) { return; }
		if ( !ParseEntitiesXmlNode( *root ) ) { return; }
		
		isValid = true;
	}
}


//-----------------------------------------------------------------------------------------------
MapDefinition::~MapDefinition()
{	
}


//-----------------------------------------------------------------------------------------------
bool MapDefinition::ParseMapDefinitionXmlNode( const XmlElement& mapDefElem )
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

	backgroundSpritePath = ParseXmlAttribute( mapDefElem, "backgroundSprite", "" );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool MapDefinition::ParseEntitiesXmlNode( const XmlElement& mapDefElem )
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

			hasParsedPlayerStart = true;
		}
		else if ( !strcmp( entityElem->Value(), "Entity" ) )
		{
			CreateMapEntityDefFromXmlNode( *entityElem );
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
void MapDefinition::CreateMapEntityDefFromXmlNode( const XmlElement& entityElem )
{
	EntitySpawnParams mapEntitySpawnParams;

	std::string entityType = ParseXmlAttribute( entityElem, "type", "" );
	mapEntitySpawnParams.entityDef = EntityTypeDefinition::GetEntityDefinition( entityType );
	if ( mapEntitySpawnParams.entityDef == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s': Entity '%s' was not defined in EntityTypes.xml", mapName.c_str(), entityType.c_str() ) );
		return;
	}

	mapEntitySpawnParams.name = ParseXmlAttribute( entityElem, "name", "" );
	mapEntitySpawnParams.position = ParseXmlAttribute( entityElem, "pos", Vec3::ZERO );

	// Parse zephyr variable defs
	const XmlElement* globalVarElem = entityElem.FirstChildElement( "GlobalVar" );
	const std::string errorStr = ParseZephyrVarInitsFromData( globalVarElem, mapEntitySpawnParams.zephyrScriptInitialValues, mapEntitySpawnParams.zephyrEntityVarInits );

	if ( !errorStr.empty() )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s': %s", mapName.c_str(), errorStr.c_str() ) );
		return;
	}

	mapEntitiesSpawnParams.push_back( mapEntitySpawnParams );
}

