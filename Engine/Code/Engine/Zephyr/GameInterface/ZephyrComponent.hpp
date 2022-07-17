#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
class ZephyrScriptDefinition;
class ZephyrEntity;


//-----------------------------------------------------------------------------------------------
class ZephyrComponent
{
public:
	ZephyrComponent( const ZephyrScriptDefinition& scriptDef, ZephyrEntity* parentEntity );
	~ZephyrComponent();

	void UnloadScript();

	// TODO: Move behaviour to a system?
	void Update();
	bool FireEvent( const std::string& eventName, EventArgs* args = nullptr );
	void ChangeState( const std::string& targetState );

	// Initialization
	void InterpretGlobalBytecodeChunk();
	void InitializeEntityVariables();
	void InitializeGlobalVariables( const ZephyrValueMap& intialValues );
	void SetEntityVariableInitializers( const std::vector<EntityVariableInitializer>& entityVarInits );

	bool IsScriptValid() const;
	void SetScriptObjectValidity( bool isValid );

	// Accessors
	std::string GetScriptName() const													{ return m_name; }
	ZephyrValue GetGlobalVariable( const std::string& varName );
	void		SetGlobalVariable( const std::string& varName, const ZephyrValue& value );
	//void		SetGlobalVec2Member( const std::string& varName, const std::string& memberName, const ZephyrValue& value );

	const ZephyrBytecodeChunk* GetBytecodeChunkByName( const std::string& chunkName ) const;

private:
	ZephyrBytecodeChunk* GetStateBytecodeChunk( const std::string& stateName );
	ZephyrBytecodeChunk* GetEventBytecodeChunk( const std::string& eventName );

private:
	bool m_isScriptObjectValid = true;
	bool m_hasEnteredStartingState = false;

	// REFACTOR: Used for error reporting, move to central data repo in ZephyrSystem or HashedString?
	std::string m_name;
	ZephyrEntity* m_parentEntity = nullptr;

	// Initial values for entity variables are given as names but must be translated into ids after all entities are loaded
	std::vector<EntityVariableInitializer> m_entityVarInits;

	// REFACTOR: Don't need to save this as all data can be copied/used to initialize this class and then isn't needed
	const ZephyrScriptDefinition& m_scriptDef;
	ZephyrBytecodeChunk* m_globalBytecodeChunk = nullptr;
	ZephyrBytecodeChunk* m_curStateBytecodeChunk = nullptr;
	ZephyrBytecodeChunkMap m_stateBytecodeChunks; 
};


// Notes for Refactor
// - Create a ZephyrStateSystem? to run the update methods for each ZephyrComponent in a scene/map, this class becomes ZephyrComponent
// - ZephyrEventSystem? takes in EntityId with each call and runs the appropriate method on given entity. Needs to interface with global entity manager somewhere
// - ZephyrGameEvents should always route calls through ZephyrEventSystem instead of Game or grabbing the entities directly
// - Find a better way to define ZephyrEvents, maybe register them directly in ZephyrEventSystem? ZephyrEventSystem could replace the g_gameAPI altogether actually 
//   as really the ZephyrGameEvents file are just wrappers to manage params and call into other functions. In that case, The ZES could be an interface for how to create
//   the system and then Game code would actually add the events and functionality. Maybe divide behaviour that is intrinsic to Zephyr (change state, debug print, etc.) into an engine
//   system while Game code defines specifics. Could even have policies to add behaviour to ZES, with the policies defined in Game code since that won't change at runtime...



// Current
// ZephyrSystem
// - Manages timers
//
// ZephyrComponent
// - All logic and script bytecode chunk data
// 
// ZephyrEntityDefinition
// - Parses initial vars for entity, reloads script def
//
// ZephyrScriptDefinition
// - Holds all bytecode chunks for ZephyrScript
//
// ZephyrEntity
// - Holds EntityId and name
// - Essentially a wrapper around calls to ZephyrComponent
//
// Entity is a child of ZephyrEntity
// - Calls Update and Die from ZephyrEntity

// Future
// Entity
// - Holds EntityId and name? HashedString at least
// - Move to Engine? Just hold id in that case and each game would inherit from Entity. Which is essentially what ZephyrEntity is today... 
//
// ZephyrEntity will be removed
//
// ZephyrComponent
// - Holds all data in ZephyrComponent and ZephyrEntityDefinition
// - Parsed from data to grab EntityDef data
//
// 
//
//
//
//
//
//
//
//
//
//