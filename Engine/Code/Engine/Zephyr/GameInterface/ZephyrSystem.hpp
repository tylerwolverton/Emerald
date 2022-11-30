#pragma once
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct ZephyrScene;
struct ZephyrComponentDefinition;
class ZephyrComponent;

class Entity;


//-----------------------------------------------------------------------------------------------
class ZephyrSystem
{
	friend struct ZephyrScene;

public:
	static void							InitializeAllZephyrEntityVariables( ZephyrScene& scene );
	static void							InitializeZephyrEntityVariables( ZephyrComponent* zephyrComp );
	static void							InitializeZephyrEntityVariables( const EntityId& entityId );
	static void							InitializeGlobalVariables( ZephyrComponent* zephyrComp, const ZephyrValueMap& initialValues );
	static void							InitializeGlobalVariables( const EntityId& entityId, const ZephyrValueMap& initialValues );

	static ZephyrValue					GetGlobalVariable( ZephyrComponent* zephyrComp, const std::string& varName );
	static ZephyrValue					GetGlobalVariable( const EntityId& entityId, const std::string& varName );
	static void							SetGlobalVariable( ZephyrComponent* zephyrComp, const std::string& varName, const ZephyrValue& value );
	static void							SetGlobalVariable( const EntityId& entityId, const std::string& varName, const ZephyrValue& value );

	static void							UnloadZephyrScripts( ZephyrScene& scene );
	static void							ReloadZephyrScripts( ZephyrScene& scene );

	static const ZephyrBytecodeChunk*	GetBytecodeChunkByName( ZephyrComponent* zephyrComp, const std::string& chunkName );
	static const ZephyrBytecodeChunk*	GetBytecodeChunkByName( const EntityId& entityId, const std::string& chunkName );

	// Logic
	static void							UpdateScene( ZephyrScene& scene );

	static void							FireAllSpawnEvents( ZephyrScene& scene );
	static void							FireSpawnEvent( ZephyrComponent* zephyrComp );
	static void							FireSpawnEvent( const EntityId& entityId );
	static bool							FireScriptEvent( ZephyrComponent* zephyrComp, const std::string& eventName, EventArgs* args = nullptr );
	static bool							FireScriptEvent( const EntityId& entityId, const std::string& eventName, EventArgs* args = nullptr );
	static void							ChangeZephyrScriptState( ZephyrComponent* zephyrComp, const std::string& targetState );
	static void							ChangeZephyrScriptState( const EntityId& entityId, const std::string& targetState );

private:
	static ZephyrComponent*				CreateComponent( Entity* parentEntity, const ZephyrComponentDefinition& componentDef );
	static void							UpdateComponent( ZephyrComponent* zephyrComp );
	static void							UnloadZephyrScript( ZephyrComponent* zephyrComp );
	static void							ReloadZephyrScript( ZephyrComponent* zephyrComp );
};
