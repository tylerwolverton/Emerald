#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class EntityTypeDefinition;
class MapRegionTypeDefinition;
class Map;


//-----------------------------------------------------------------------------------------------
struct MapEntityDefinition
{
	EntityTypeDefinition* entityDef = nullptr;
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
	explicit MapDefinition( const std::string& mapFullPath );
	~MapDefinition();
	
private:
	bool ParseMapDefinitionXmlNode( const XmlElement& mapDefElem );
	bool ParseEntitiesXmlNode( const XmlElement& mapDefElem );
	void CreateMapEntityDefFromXmlNode( const XmlElement& entityElem );
};
