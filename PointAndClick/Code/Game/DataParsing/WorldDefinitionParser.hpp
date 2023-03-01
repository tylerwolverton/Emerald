#pragma once
#include <map>
#include <string>


//-----------------------------------------------------------------------------------------------
class EntityTypeDefinition;


//-----------------------------------------------------------------------------------------------
class WorldDefinitionParser
{
public:
	static std::map<std::string, EntityTypeDefinition*> ParseEntitiesFromFile( const std::string& fullPath );

private:
	static std::map<std::string, EntityTypeDefinition*> ParseEntitiesFromXmlFile( const std::string& fullPath );
};
