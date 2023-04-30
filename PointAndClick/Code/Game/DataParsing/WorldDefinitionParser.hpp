#pragma once
#include "Game/Core/GameCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class EntityTypeDefinition;


//-----------------------------------------------------------------------------------------------
class WorldDefinitionParser
{
public:
	static std::vector<EntitySpawnParams> ParseEntitiesFromFile( const std::string& fullPath );

private:
	static std::vector<EntitySpawnParams> ParseEntitiesFromXmlFile( const std::string& fullPath );
};
