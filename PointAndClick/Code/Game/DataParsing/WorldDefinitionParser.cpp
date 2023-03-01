#include "Game/DataParsing/WorldDefinitionParser.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"

#include "Game/Core/GameCommon.hpp"
#include "Game/DataParsing/EntityTypeDefinition.hpp"


//-----------------------------------------------------------------------------------------------
std::map<std::string, EntityTypeDefinition*> WorldDefinitionParser::ParseEntitiesFromFile( const std::string& fullPath )
{
	std::string fileExt = GetFileExtension( fullPath );
	if ( IsEqualIgnoreCase( fileExt, ".xml" ) )
	{
		return ParseEntitiesFromXmlFile( fullPath );
	}

	return std::map<std::string, EntityTypeDefinition*>();
}


//-----------------------------------------------------------------------------------------------
std::map<std::string, EntityTypeDefinition*> WorldDefinitionParser::ParseEntitiesFromXmlFile( const std::string& fullPath )
{
	std::map<std::string, EntityTypeDefinition*> worldEntityMap;

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( fullPath.c_str() );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( Stringf( "The world xml file '%s' could not be opened.", fullPath.c_str() ) );
		return worldEntityMap;
	}

	XmlElement* root = doc.RootElement();
	if ( strcmp( root->Name(), "WorldDefinition" ) )
	{
		g_devConsole->PrintError( Stringf( "'%s': Incorrect root node name, must be WorldDefinition", fullPath.c_str() ) );
		return worldEntityMap;
	}

	// Parse entities node
	XmlElement* entitiesElement = root->FirstChildElement( "Entities" );
	XmlElement* entityElement = entitiesElement->FirstChildElement();
	while ( entityElement )
	{
		if ( !strcmp( entityElement->Name(), "Entity" ) )
		{
			std::string entityTypeStr = ParseXmlAttribute( *entityElement, "type", "" );
			if ( entityTypeStr.empty() )
			{
				g_devConsole->PrintError( Stringf( "'%s': %s is missing a type attribute", fullPath.c_str(), entityElement->Name() ) );
				entityElement = entityElement->NextSiblingElement();
				continue;
			}

			EntityTypeDefinition* entityTypeDef = EntityTypeDefinition::GetEntityDefinition( entityTypeStr );
			if ( entityTypeDef == nullptr )
			{
				g_devConsole->PrintError( Stringf( "'%s': Entity type '%s' was not defined in EntityTypes.xml", fullPath.c_str(), entityTypeStr.c_str() ) );
				entityElement = entityElement->NextSiblingElement();
				continue;
			}

			std::string entityName = ParseXmlAttribute( *entityElement, "name", "" );

			worldEntityMap[entityName] = entityTypeDef;
		}
		else
		{
			g_devConsole->PrintError( Stringf( "WorldDef.xml: Unsupported node '%s'", entityElement->Name() ) );
		}

		entityElement = entityElement->NextSiblingElement();
	}

	return worldEntityMap;
}