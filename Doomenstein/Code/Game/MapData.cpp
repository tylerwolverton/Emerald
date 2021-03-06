#include "Game/MapData.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/Transform.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/MapRegionTypeDefinition.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
MapData::MapData( const XmlElement& mapDefElem, const std::string& mapName, const std::string& defaultRegionName )
	: mapName( mapName )
{
	if ( !ParseMapDefinitionNode( mapDefElem ) ) { return; }

	if ( type == "TileMap" )
	{
		std::map<char, MapRegionTypeDefinition*> legend;
		if ( !ParseLegendNode( mapDefElem, legend, defaultRegionName ) ) { return; }
		if ( !ParseMapRowsNode( mapDefElem, legend, defaultRegionName ) ) { return; }
	}
	else if ( type == "LineMap" )
	{
		if ( !ParseWalls( mapDefElem, defaultRegionName ) ) { return; }
	}

	if ( !ParseEntitiesNode( mapDefElem ) ) { return; }

	isValid = true;
}


//-----------------------------------------------------------------------------------------------
MapData::~MapData()
{	
}


//-----------------------------------------------------------------------------------------------
bool MapData::ParseMapDefinitionNode( const XmlElement& mapDefElem )
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

	dimensions = ParseXmlAttribute( mapDefElem, "dimensions", dimensions );
	if ( dimensions == IntVec2::ZERO
		 && type == "TileMap" )
	{
		g_devConsole->PrintError( Stringf( "Tile Map file '%s' is missing a dimensions attribute", mapName.c_str() ) );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool MapData::ParseLegendNode( const XmlElement& mapDefElem, std::map<char, MapRegionTypeDefinition*>& legend, const std::string& defaultRegionName )
{
	regionTypeDefs.resize( (size_t)dimensions.x * (size_t)dimensions.y );

	const XmlElement* legendElem = mapDefElem.FirstChildElement( "Legend" );
	if ( legendElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' is missing a legend", mapName.c_str() ) );
		return false;
	}

	const XmlElement* tileElem = legendElem->FirstChildElement( "Tile" );
	if ( tileElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' has no Tile nodes in legend", mapName.c_str() ) );
	}

	while ( tileElem )
	{
		char glyph = ParseXmlAttribute( *tileElem, "glyph", ' ' );
		if ( legend.find( glyph ) != legend.end() )
		{
			g_devConsole->PrintError( Stringf( "Glyph '%c' has already been defined in legend, first definition will be used", glyph ) );
			tileElem = tileElem->NextSiblingElement();
			continue;
		}

		std::string regionTypeStr = ParseXmlAttribute( *tileElem, "regionType", "" );
		MapRegionTypeDefinition* regionDef = nullptr;
		if ( regionTypeStr == "" )
		{
			g_devConsole->PrintError( Stringf( "Glyph '%c' is missing a regionType attribute in legend", glyph ) );
			regionDef = MapRegionTypeDefinition::GetMapRegionTypeDefinition( defaultRegionName );
		}
		else
		{
			regionDef = MapRegionTypeDefinition::GetMapRegionTypeDefinition( regionTypeStr );
			if ( regionDef == nullptr )
			{
				g_devConsole->PrintError( Stringf( "Glyph '%c' references unknown regionType '%s'", glyph, regionTypeStr.c_str() ) );
				regionDef = MapRegionTypeDefinition::GetMapRegionTypeDefinition( defaultRegionName );
			}
		}

		legend[glyph] = regionDef;

		tileElem = tileElem->NextSiblingElement();
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool MapData::ParseWalls( const XmlElement& mapDefElem, const std::string& defaultRegionName )
{
	UNUSED( defaultRegionName );

	const XmlElement* wallsElem = mapDefElem.FirstChildElement( "Walls" );
	if ( wallsElem == nullptr )
	{
		g_devConsole->PrintError( "No Walls element defined" );
		return false;
	}

	const XmlElement* wallElem = wallsElem->FirstChildElement( "Wall" );
	while ( wallElem )
	{
		Vec3 startPos = ParseXmlAttribute( *wallElem, "startPos", Vec3::ZERO );
		Vec3 endPos = ParseXmlAttribute( *wallElem, "endPos", Vec3( 1.f, 0.f, 0.f ) );
		float height = ParseXmlAttribute( *wallElem, "height", 1.f );
		float width = ParseXmlAttribute( *wallElem, "width", 1.f );
		
		std::string regionTypeStr = ParseXmlAttribute( *wallElem, "regionType", defaultRegionName );
		MapRegionTypeDefinition* regionDef = MapRegionTypeDefinition::GetMapRegionTypeDefinition( regionTypeStr );
	
		if ( regionDef == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Wall references unknown regionType '%s'", regionTypeStr.c_str() ) );
			regionDef = MapRegionTypeDefinition::GetMapRegionTypeDefinition( defaultRegionName );
		}
		
		OBB3 wall = ConstructWall( startPos, endPos, height, width );

		walls.emplace_back( wall );
		regionTypeDefs.push_back( regionDef );

		wallElem = wallElem->NextSiblingElement();
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
OBB3 MapData::ConstructWall( const Vec3& startPos, const Vec3& endPos, float height, float width )
{
	// Determine points looking in the direction of the specified start and end, with height going up and width going to the right
	Mat44 basisMatrix = MakeLookAtMatrix( startPos, endPos, Vec3( 0.f, 0.f, 1.f ) );

	float depth = GetDistance3D( startPos, endPos );

	Vec3 center = startPos
		+ ( basisMatrix.GetIBasis3D() * ( width * .5f ) )
		+ ( basisMatrix.GetJBasis3D() * ( height * .5f ) )
		- ( basisMatrix.GetKBasis3D() * ( depth * .5f ) );

	Vec3 fullDimensions( width, height, depth );

	return OBB3( center, fullDimensions, basisMatrix.GetIBasis3D(), basisMatrix.GetJBasis3D() );
}


//-----------------------------------------------------------------------------------------------
bool MapData::ParseMapRowsNode( const XmlElement& mapDefElem, const std::map<char, MapRegionTypeDefinition*>& legend, const std::string& defaultRegionName )
{
	const XmlElement* mapRowsElem = mapDefElem.FirstChildElement( "MapRows" );
	if ( mapRowsElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' has no MapRows node", mapName.c_str() ) );
		return false;
	}

	const XmlElement* mapRowElem = mapRowsElem->FirstChildElement( "MapRow" );
	if ( mapRowsElem == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map file '%s' has no MapRow nodes defined", mapName.c_str() ) );
		return false;
	}

	int rowNum = dimensions.y - 1;
	int numRowsDefined = 0;
	while ( mapRowElem )
	{
		++numRowsDefined;
		if ( numRowsDefined > dimensions.y )
		{
			g_devConsole->PrintError( Stringf( "Map '%s': Too many MapRows defined. Expected: %d", mapName.c_str(), dimensions.y ) );
			return false;
		}

		std::string tilesStr = ParseXmlAttribute( *mapRowElem, "tiles", "" );
		if ( tilesStr == "" )
		{
			g_devConsole->PrintError( Stringf( "Map '%s': MapRow %d is missing a tiles attribute", mapName.c_str(), numRowsDefined ) );
			return false;
		}

		if ( (int)tilesStr.size() != dimensions.x )
		{
			g_devConsole->PrintError( Stringf( "Map '%s': MapRow %d has incorrect number of tiles defined. Expected: %d  Actual: %d", mapName.c_str(), numRowsDefined, dimensions.x, (int)tilesStr.size() ) );
			return false;
		}

		for ( int regionDefNum = 0; regionDefNum < tilesStr.length(); ++regionDefNum )
		{
			MapRegionTypeDefinition* regionDef = nullptr;
			auto it = legend.find( tilesStr[regionDefNum] );
			if ( it == legend.end() )
			{
				g_devConsole->PrintError( Stringf( "Map '%s': MapRow %d contains unknown glyph '%c'", mapName.c_str(), numRowsDefined, tilesStr[regionDefNum] ) );
				regionDef = MapRegionTypeDefinition::GetMapRegionTypeDefinition( defaultRegionName );
			}
			else
			{
				regionDef = it->second;
			}

			int tileIdx = ( rowNum * dimensions.x ) + regionDefNum;
			regionTypeDefs[tileIdx] = regionDef;
		}

		mapRowElem = mapRowElem->NextSiblingElement();
		--rowNum;
	}

	if ( numRowsDefined < dimensions.y )
	{
		g_devConsole->PrintError( Stringf( "Map '%s': Not enough MapRows defined. Expected: %d  Actual: %d", mapName.c_str(), dimensions.y, numRowsDefined ) );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool MapData::ParseEntitiesNode( const XmlElement& mapDefElem )
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
		else
		{
			CreateMapEntityDefFromNode( *entityElem );
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
void MapData::CreateMapEntityDefFromNode( const XmlElement& entityElem )
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

	mapEntityDef.position = ParseXmlAttribute( entityElem, "pos", Vec3::ZERO );
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

