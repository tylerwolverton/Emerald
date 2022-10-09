#pragma once
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct ZephyrScene;
class ZephyrComponent;
class ZephyrComponentDefinition;

class Entity;


//-----------------------------------------------------------------------------------------------
class ZephyrSystem
{
	friend struct ZephyrScene;

public:
	static void							InitializeAllZephyrEntityVariables( ZephyrScene& scene );
	static void							InitializeGlobalVariables( ZephyrComponent* zephyrComp, const ZephyrValueMap& initialValues );

	static ZephyrValue					GetGlobalVariable( ZephyrComponent* zephyrComp, const std::string& varName );
	static ZephyrValue					GetGlobalVariable( EntityId entityId, const std::string& varName );
	static void							SetGlobalVariable( ZephyrComponent* zephyrComp, const std::string& varName, const ZephyrValue& value );

	static void							UnloadZephyrScripts( ZephyrScene& scene );
	static void							ReloadZephyrScripts( ZephyrScene& scene );

	static const ZephyrBytecodeChunk*	GetBytecodeChunkByName( ZephyrComponent* zephyrComp, const std::string& chunkName );

	// Logic
	static void							UpdateScene( ZephyrScene& scene );
	
	static void							FireSpawnEvent( ZephyrComponent* zephyrComp );
	static bool							FireScriptEvent( ZephyrComponent* zephyrComp, const std::string& eventName, EventArgs* args = nullptr );
	static void							ChangeZephyrScriptState( ZephyrComponent* zephyrComp, const std::string& targetState );

private:
	static ZephyrComponent*				CreateComponent( Entity* parentEntity, const ZephyrComponentDefinition& componentDef );
	static void							UpdateComponent( ZephyrComponent* zephyrComp );
	static void							UnloadZephyrScript( ZephyrComponent* zephyrComp );
	static void							ReloadZephyrScript( ZephyrComponent* zephyrComp );
	static void							InitializeZephyrEntityVariables( ZephyrComponent* zephyrComp );
};
