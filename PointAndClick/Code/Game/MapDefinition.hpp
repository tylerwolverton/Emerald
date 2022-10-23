#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
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
struct MapDefinition
{
public:
	bool isValid = false;									// Game
	std::string mapName;									// Map
	std::string type = "InvalidType";						// World
	int version = -1;										

	std::vector<MapEntityDefinition> mapEntityDefs;			// Map

	// Multiplayer TODO: Make this into an array
	Vec3 playerStartPos = Vec3::ZERO;
	float playerStartYaw = 0.f;

public:
	explicit MapDefinition( const XmlElement& mapDefElem, const std::string& mapName );
	~MapDefinition();
	
private:
	bool ParseMapDefinitionNode( const XmlElement& mapDefElem );
	bool ParseEntitiesNode( const XmlElement& mapDefElem );
	void CreateMapEntityDefFromNode( const XmlElement& entityElem );
};
