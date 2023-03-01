#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
class EntityTypesParser
{
public:
	static void ParseFromFile( const std::string& fullPath );

private:
	static void ParseFromXmlFile( const std::string& fullPath );
};
