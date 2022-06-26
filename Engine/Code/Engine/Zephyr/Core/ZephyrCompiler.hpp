#pragma once
#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrScriptDefinition;


//-----------------------------------------------------------------------------------------------
class ZephyrCompiler
{
public:
	static ZephyrScriptDefinition* CompileScriptFile( const std::string& filePath );
};
