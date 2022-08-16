#include "Engine/Zephyr/GameInterface/ZephyrComponent.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Zephyr/Core/ZephyrBytecodeChunk.hpp"
#include "Engine/Zephyr/Core/ZephyrScriptDefinition.hpp"
#include "Engine/Zephyr/Core/ZephyrInterpreter.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrEngineEvents.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrEntity.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrComponent::ZephyrComponent( const ZephyrScriptDefinition& scriptDef, ZephyrEntity* parentEntity )
	: m_name( scriptDef.m_name )
	, m_parentEntity( parentEntity )
{
	m_isScriptObjectValid = scriptDef.IsValid();
	if ( !m_isScriptObjectValid )
	{
		return;
	}
	
	if ( parentEntity == nullptr )
	{
		return;
	}

	m_globalBytecodeChunk = new ZephyrBytecodeChunk( *scriptDef.GetGlobalBytecodeChunk() );
	GUARANTEE_OR_DIE( m_globalBytecodeChunk != nullptr, "Global Bytecode Chunk was null" );
	
	m_curStateBytecodeChunk = scriptDef.GetFirstStateBytecodeChunk();
	m_stateBytecodeChunks = scriptDef.GetAllStateBytecodeChunks();

	// Initialize parentEntity in script
	m_globalBytecodeChunk->SetVariable( PARENT_ENTITY_NAME, ZephyrValue( (EntityId)m_parentEntity->GetId() ) );
}


//-----------------------------------------------------------------------------------------------
ZephyrComponent::~ZephyrComponent()
{
	g_eventSystem->DeRegisterObject( this );

	PTR_SAFE_DELETE( m_globalBytecodeChunk );
}


//-----------------------------------------------------------------------------------------------
void ZephyrComponent::UnloadScript()
{
	if ( !IsScriptValid() )
	{
		return;
	}

	m_stateBytecodeChunks.clear();
}


//-----------------------------------------------------------------------------------------------
bool ZephyrComponent::FireEvent( const std::string& eventName, EventArgs* args )
{
	if ( !IsScriptValid() )
	{
		return false;
	}

	EventArgs eventArgs;
	if ( args == nullptr )
	{
		args = &eventArgs;
	}

	ZephyrBytecodeChunk* eventChunk = GetEventBytecodeChunk( eventName );
	if ( eventChunk == nullptr )
	{
		return false;
	}

	ZephyrValueMap* stateVariables = nullptr;
	if ( m_curStateBytecodeChunk != nullptr )
	{
		stateVariables = m_curStateBytecodeChunk->GetUpdateableVariables();
	}

	m_parentEntity->AddGameEventParams( args );
		
	ZephyrInterpreter::InterpretEventBytecodeChunk( *eventChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity, args, stateVariables );
	return true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrComponent::ChangeState( const std::string& targetState )
{
	if ( !IsScriptValid() )
	{
		return;
	}

	ZephyrBytecodeChunk* targetStateBytecodeChunk = GetStateBytecodeChunk( targetState );
	if ( targetStateBytecodeChunk == nullptr 
		 || targetStateBytecodeChunk == m_curStateBytecodeChunk )
	{
		// State doesn't exist, should be reported by compiler to avoid flooding with errors here
		// Or the state change is a no-op
		return;
	}

	FireEvent( "OnExit" );
	
	m_curStateBytecodeChunk = targetStateBytecodeChunk;
	// Initialize state variables each time the state is entered
	ZephyrInterpreter::InterpretStateBytecodeChunk( *m_curStateBytecodeChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity, m_curStateBytecodeChunk->GetUpdateableVariables() );

	FireEvent( "OnEnter" );
	m_hasEnteredStartingState = true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrComponent::InterpretGlobalBytecodeChunk()
{
	ZephyrInterpreter::InterpretStateBytecodeChunk( *m_globalBytecodeChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity );
	
	// Initialize default state variables
	if ( m_curStateBytecodeChunk != nullptr )
	{
		ZephyrInterpreter::InterpretStateBytecodeChunk( *m_curStateBytecodeChunk, m_globalBytecodeChunk->GetUpdateableVariables(), m_parentEntity, m_curStateBytecodeChunk->GetUpdateableVariables() );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrComponent::SetEntityVariableInitializers( const std::vector<EntityVariableInitializer>& entityVarInits )
{
	m_entityVarInits.insert( m_entityVarInits.begin(), entityVarInits.begin(), entityVarInits.end() );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrComponent::IsScriptValid() const
{
	return m_isScriptObjectValid;
}


//-----------------------------------------------------------------------------------------------
void ZephyrComponent::SetScriptObjectValidity( bool isValid )
{
	m_isScriptObjectValid = isValid;
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
		ZephyrEntity* entity = g_zephyrAPI->GetEntityByName( entityVarInit.entityName );
		if ( entity == nullptr )
		{
			g_devConsole->PrintError( Stringf( "Error defining entity variable '%s' in zephyr script. Entity with name '%s' can not be found", entityVarInit.varName.c_str(), entityVarInit.entityName.c_str() ) );
			m_isScriptObjectValid = false;
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
