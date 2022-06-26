#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include "Game/MapRegionTypeDefinition.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class TileDefinition;
class EntityDefinition;
class MapRegionTypeDefinition;
class Map;


//-----------------------------------------------------------------------------------------------
struct MapEntityDefinition
{
	EntityDefinition* entityDef = nullptr;
	std::string name;
	Vec3 position = Vec3::ZERO;
	float yawDegrees = 0.f;

	ZephyrValueMap zephyrScriptInitialValues;
	std::vector<EntityVariableInitializer> zephyrEntityVarInits;
};


//-----------------------------------------------------------------------------------------------
struct MapData
{
public:
	bool isValid = false;									// Game
	std::string mapName;									// Map
	std::string type = "InvalidType";						// World
	int version = -1;										
	IntVec2 dimensions = IntVec2::ZERO;						// TileMap

	std::vector<MapEntityDefinition> mapEntityDefs;			// Map
	std::vector<MapRegionTypeDefinition*> regionTypeDefs;	// TileMap and LineMap

	std::vector<OBB3> walls;								// LineMap

	// Multiplayer TODO: Make this into an array
	Vec3 playerStartPos = Vec3::ZERO;
	float playerStartYaw = 0.f;

public:
	explicit MapData( const XmlElement& mapDefElem, const std::string& mapName, const std::string& defaultRegionName );
	~MapData();
	
private:
	bool ParseMapDefinitionNode( const XmlElement& mapDefElem );
	bool ParseLegendNode( const XmlElement& mapDefElem, std::map<char, MapRegionTypeDefinition*>& legend, const std::string& defaultRegionName );
	bool ParseWalls( const XmlElement& mapDefElem, const std::string& defaultRegionName );
	OBB3 ConstructWall( const Vec3& startPos, const Vec3& endPos, float height, float width );
	bool ParseMapRowsNode( const XmlElement& mapDefElem, const std::map<char, MapRegionTypeDefinition*>& legend, const std::string& defaultRegionName );
	bool ParseEntitiesNode( const XmlElement& mapDefElem );
	void CreateMapEntityDefFromNode( const XmlElement& entityElem );
};
