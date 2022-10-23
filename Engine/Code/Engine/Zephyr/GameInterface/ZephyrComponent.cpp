#include "Engine/Zephyr/GameInterface/ZephyrComponent.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Framework/Entity.hpp"
#include "Engine/Zephyr/Core/ZephyrBytecodeChunk.hpp"
#include "Engine/Zephyr/Core/ZephyrScriptDefinition.hpp"
#include "Engine/Zephyr/Core/ZephyrInterpreter.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrEngineEvents.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponentDefinition.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrComponent::ZephyrComponent( const ZephyrComponentDefinition& componentDef, const EntityId& parentEntityId )
	: m_componentDef( componentDef )
	, m_parentEntityId( parentEntityId )
{
	m_typeId = ENTITY_COMPONENT_TYPE_ZEPHYR;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrComponent::Initialize()
{
	ZephyrScriptDefinition* scriptDef = m_componentDef.GetZephyrScriptDefinition();
	if ( scriptDef == nullptr || !scriptDef->IsValid() )
	{
		m_compState = eComponentState::INVALID_SCRIPT;
		return false;
	}

	if ( m_parentEntityId == INVALID_ENTITY_ID )
	{
		m_compState = eComponentState::INVALID_PARENT;
		return false;
	}

	m_globalBytecodeChunk = new ZephyrBytecodeChunk( *scriptDef->GetGlobalBytecodeChunk() );
	GUARANTEE_OR_DIE( m_globalBytecodeChunk != nullptr, "Global Bytecode Chunk was null" );

	m_curStateBytecodeChunk = scriptDef->GetFirstStateBytecodeChunk();
	m_stateBytecodeChunks = scriptDef->GetAllStateBytecodeChunks();

	// Initialize parentEntity in script
	m_globalBytecodeChunk->SetVariable( PARENT_ENTITY_STR, ZephyrValue( m_parentEntityId ) );

	m_compState = eComponentState::INITIALIZED;
	return true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrComponent::Destroy()
{
	g_eventSystem->DeRegisterObject( this );

	PTR_SAFE_DELETE( m_globalBytecodeChunk );

	m_compState = eComponentState::UNINITIALIZED;
}


//-----------------------------------------------------------------------------------------------
ZephyrComponent::~ZephyrComponent()
{
	Destroy();
}


//-----------------------------------------------------------------------------------------------
// TODO: Why are we interpreting current chunk (to initialize variables defined in state) and should the system do this?
void ZephyrComponent::InterpretGlobalBytecodeChunk()
{
	ZephyrInterpreter::InterpretStateBytecodeChunk( *m_globalBytecodeChunk, m_globalBytecodeChunk->GetUpdateableVariables(), *this );
	
	// Initialize default state variables
	if ( m_curStateBytecodeChunk != nullptr )
	{
		ZephyrInterpreter::InterpretStateBytecodeChunk( *m_curStateBytecodeChunk, m_globalBytecodeChunk->GetUpdateableVariables(), *this, m_curStateBytecodeChunk->GetUpdateableVariables() );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrComponent::SetEntityVariableInitializers( const std::vector<EntityVariableInitializer>& entityVarInits )
{
	m_entityVarInits.insert( m_entityVarInits.begin(), entityVarInits.begin(), entityVarInits.end() );
}


//-----------------------------------------------------------------------------------------------
EntityId ZephyrComponent::GetParentEntityId() const
{
	return m_parentEntityId;
}


//-----------------------------------------------------------------------------------------------
std::string ZephyrComponent::GetParentEntityName() const
{
	if ( !IsScriptValid()
		 || m_parentEntityId == INVALID_ENTITY_ID )
	{
		//g_devConsole->PrintError( "Invalid zephyr component doesn't have valid parent name" );
		return "Unknown";
	}

	return Entity::GetName( m_parentEntityId );
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrComponent::GetGlobalVariable( const std::string& varName )
{
	ZephyrValue val( ERROR_ZEPHYR_VAL );

	m_globalBytecodeChunk->TryToGetVariable( varName, val );

	return val;
}


//-----------------------------------------------------------------------------------------------
void ZephyrComponent::SetGlobalVariable( const std::string& varName, const ZephyrValue& value )
{
	m_globalBytecodeChunk->SetVariable( varName, value );
}


//-----------------------------------------------------------------------------------------------
void ZephyrComponent::InitializeEntityVariables()
{
	ZephyrValueMap validEntities;
	
	for ( const auto& entityVarInit : m_entityVarInits )
	{
		Entity* entity = g_zephyrAPI->GetEntityByName( entityVarInit.entityName );
		if ( entity == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Error defining entity variable '%s' in zephyr script. Entity with name '%s' can not be found", entityVarInit.varName.c_str(), entityVarInit.entityName.c_str() ) );
			m_compState = eComponentState::INVALID_SCRIPT;
			continue;
		}

		validEntities[entityVarInit.varName] = entity->GetId();
	}

	ZephyrSystem::InitializeGlobalVariables( this, validEntities );
}


//-----------------------------------------------------------------------------------------------
const ZephyrBytecodeChunk* ZephyrComponent::GetBytecodeChunkByName( const std::string& chunkName ) const
{
	if ( m_globalBytecodeChunk->GetName() == chunkName )
	{
		return m_globalBytecodeChunk;
	}

	// Check for global functions
	for ( const auto& eventPair : m_globalBytecodeChunk->GetEventBytecodeChunks() )
	{
		if ( eventPair.second->GetName() == chunkName )
		{
			return eventPair.second;
		}
	}

	for ( const auto& statePair : m_stateBytecodeChunks )
	{
		if ( statePair.second->GetName() == chunkName )
		{
			return statePair.second;
		}

		for ( const auto& eventPair : statePair.second->GetEventBytecodeChunks() )
		{
			if ( eventPair.second->GetName() == chunkName )
			{
				return eventPair.second;
			}
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk* ZephyrComponent::GetStateBytecodeChunk( const std::string& stateName )
{
	ZephyrBytecodeChunkMap::const_iterator  mapIter = m_stateBytecodeChunks.find( stateName );

	if ( mapIter == m_stateBytecodeChunks.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk* ZephyrComponent::GetEventBytecodeChunk( const std::string& eventName )
{
	if ( m_curStateBytecodeChunk != nullptr )
	{
		ZephyrBytecodeChunkMap::const_iterator  mapIter = m_curStateBytecodeChunk->GetEventBytecodeChunks().find( eventName );

		if ( mapIter != m_curStateBytecodeChunk->GetEventBytecodeChunks().cend() )
		{
			return mapIter->second;
		}
	}

	ZephyrBytecodeChunkMap::const_iterator mapIter = m_globalBytecodeChunk->GetEventBytecodeChunks().find( eventName );

	if ( mapIter != m_globalBytecodeChunk->GetEventBytecodeChunks().cend() )
	{
		return mapIter->second;
	}

	return nullptr;
}
