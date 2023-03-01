#include "Game/DataParsing/EntityTypesParser.hpp"
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
void EntityTypesParser::ParseFromFile( const std::string& fullPath )
{
	std::string fileExt = GetFileExtension( fullPath );
	if ( IsEqualIgnoreCase( fileExt, ".xml" ) )
	{
		ParseFromXmlFile( fullPath );
	}
}


//-----------------------------------------------------------------------------------------------
void EntityTypesParser::ParseFromXmlFile( const std::string& fullPath )
{
	XmlDocument doc;
	XmlError loadError = doc.LoadFile( fullPath.c_str() );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( Stringf( "%s could not be opened", fullPath.c_str() ) );
		return;
	}

	XmlElement* root = doc.RootElement();
	if ( strcmp( root->Name(), "EntityTypes" ) )
	{
		g_devConsole->PrintError( "EntityTypes.xml: Incorrect root node name, must be EntityTypes" );
		return;
	}

	// Parse sprite sheet
	std::string spriteSheetPath = ParseXmlAttribute( *root, "spriteSheet", "" );
	if ( spriteSheetPath == "" )
	{
		g_devConsole->PrintError( "EntityTypes.xml: Missing spriteSheet attribute" );
		return;
	}

	SpriteSheet* defaultSpriteSheet = SpriteSheet::GetSpriteSheetByPath( spriteSheetPath );
	if ( defaultSpriteSheet == nullptr )
	{
		IntVec2 spriteSheetDimensions = ParseXmlAttribute( *root, "spriteSheetDimensions", IntVec2( -1, -1 ) );
		if ( spriteSheetDimensions == IntVec2( -1, -1 ) )
		{
			g_devConsole->PrintError( "EntityTypes.xml: Missing layout attribute" );
			return;
		}

		Texture* texture = g_renderer->CreateOrGetTextureFromFile( spriteSheetPath.c_str() );
		if ( texture == nullptr )
		{
			g_devConsole->PrintError( Stringf( "EntityTypes.xml: Couldn't load texture '%s'", spriteSheetPath.c_str() ) );
			return;
		}

		defaultSpriteSheet = SpriteSheet::CreateAndRegister( *texture, spriteSheetDimensions );
	}

	// Parse each entity type
	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		if ( !strcmp( element->Name(), "Entity" ) )
		{
			EntityTypeDefinition* entityTypeDef = new EntityTypeDefinition( *element, defaultSpriteSheet );
			if ( entityTypeDef->IsValid() )
			{
				EntityTypeDefinition::s_definitions[entityTypeDef->GetType()] = entityTypeDef;
			}
		}
		else
		{
			g_devConsole->PrintError( Stringf( "EntityTypes.xml: Unsupported node '%s'", element->Name() ) );
		}

		element = element->NextSiblingElement();
	}
}
