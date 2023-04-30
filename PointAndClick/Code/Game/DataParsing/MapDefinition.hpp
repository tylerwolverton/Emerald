#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Game/Core/GameCommon.hpp"

#include <string>
#include <map>


//-----------------------------------------------------------------------------------------------
class EntityTypeDefinition;
class MapRegionTypeDefinition;
class Map;


//-----------------------------------------------------------------------------------------------
struct MapDefinition
{
public:
	bool isValid = false;									
	std::string mapName;									
	std::string type = "InvalidType";						
	int version = -1;			

	std::vector<EntitySpawnParams> mapEntitiesSpawnParams;	// Map

	std::string backgroundSpritePath;						// PointAndClick Map						

	Vec3 playerStartPos = Vec3::ZERO;

public:
	explicit MapDefinition( const std::string& mapFullPath );
	~MapDefinition();
	
private:
	bool ParseMapDefinitionXmlNode( const XmlElement& mapDefElem );
	bool ParseEntitiesXmlNode( const XmlElement& mapDefElem );
	void CreateMapEntityDefFromXmlNode( const XmlElement& entityElem );
};
