#pragma once
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrComponent;
class ZephyrComponentDefinition;

class Entity;


//-----------------------------------------------------------------------------------------------
class ZephyrSystem
{
public:
	static ZephyrComponent*				CreateComponent( Entity* parentEntity, const ZephyrComponentDefinition& componentDef );

	static void							InitializeZephyrEntityVariables( ZephyrComponent* zephyrComp );
	static void							InitializeGlobalVariables( ZephyrComponent* zephyrComp, const ZephyrValueMap& initialValues );

	static ZephyrValue					GetGlobalVariable( ZephyrComponent* zephyrComp, const std::string& varName );
	static void							SetGlobalVariable( ZephyrComponent* zephyrComp, const std::string& varName, const ZephyrValue& value );

	static void							ChangeZephyrScriptState( ZephyrComponent* zephyrComp, const std::string& targetState );
	static void							UnloadZephyrScript( ZephyrComponent* zephyrComp );
	static void							ReloadZephyrScript( ZephyrComponent* zephyrComp );

	static const ZephyrBytecodeChunk*	GetBytecodeChunkByName( ZephyrComponent* zephyrComp, const std::string& chunkName );

	// Logic
	static void							UpdateComponents( std::vector<ZephyrComponent*>& components );
	static void							FireSpawnEvent( ZephyrComponent* zephyrComp );
	static bool							FireScriptEvent( ZephyrComponent* zephyrComp, const std::string& eventName, EventArgs* args = nullptr );
};
