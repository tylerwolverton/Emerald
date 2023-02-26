#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Framework/EntityComponent.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponentDefinition.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
class Entity;


//-----------------------------------------------------------------------------------------------
enum class eComponentState : uint8_t
{
	UNINITIALIZED,
	INVALID_SCRIPT,
	INVALID_PARENT,
	INITIALIZED,
	STARTED
};


//-----------------------------------------------------------------------------------------------
class ZephyrComponent : public EntityComponent
{
	friend class ZephyrSystem;
	friend class ZephyrVirtualMachine;

public:
	ZephyrComponent( const ZephyrComponentDefinition& componentDef, const EntityId& parentEntityId );
	bool Initialize();
	void Destroy();
	~ZephyrComponent();

	// Initialization
	void InterpretGlobalBytecodeChunk();
	void InitializeEntityVariables();
	void SetEntityVariableInitializers( const std::vector<EntityVariableInitializer>& entityVarInits );

	// Accessors
	std::string		GetScriptName() const													{ return m_componentDef.zephyrScriptName;	}
	ZephyrValue		GetGlobalVariable( const std::string& varName );
	void			SetGlobalVariable( const std::string& varName, const ZephyrValue& value );
	bool			IsScriptValid() const													{ return m_compState != eComponentState::INVALID_SCRIPT
																									&& m_compState != eComponentState::INVALID_PARENT
																									&& m_compState != eComponentState::UNINITIALIZED; }

	const ZephyrBytecodeChunk* GetBytecodeChunkByName( const std::string& chunkName ) const;

private:
	ZephyrBytecodeChunk* GetStateBytecodeChunk( const std::string& stateName );
	ZephyrBytecodeChunk* GetEventBytecodeChunk( const std::string& eventName );

public:
	const ZephyrComponentDefinition& m_componentDef;

	// Initial values for entity variables are given as names but must be translated into ids after all entities are loaded
	std::vector<EntityVariableInitializer> m_entityVarInits;

	ZephyrBytecodeChunk* m_globalBytecodeChunk = nullptr;
	ZephyrBytecodeChunk* m_curStateBytecodeChunk = nullptr;
	ZephyrBytecodeChunkMap m_stateBytecodeChunks;				// Duplicate here to avoid touching comp or script def at runtime. Change to ptr?

	enum class eComponentState m_compState = eComponentState::UNINITIALIZED;
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