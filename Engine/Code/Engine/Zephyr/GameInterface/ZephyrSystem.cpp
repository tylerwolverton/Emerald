#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"
#include "Engine/Zephyr/Core/ZephyrBytecodeChunk.hpp"
#include "Engine/Zephyr/Core/ZephyrInterpreter.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponent.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponentDefinition.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrScene.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Framework/Entity.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrComponent* ZephyrSystem::CreateComponent( Entity* parentEntity, const ZephyrComponentDefinition& componentDef )
{	
	ZephyrComponent* zephyrComp = new ZephyrComponent( componentDef, parentEntity );
	if ( !zephyrComp->Initialize() )
	{
		return zephyrComp;
	}

	//parentEntity->SetZephyrComponent( zephyrComp );
	zephyrComp->InterpretGlobalBytecodeChunk();
	ZephyrSystem::InitializeGlobalVariables( zephyrComp, componentDef.GetZephyrScriptInitialValues() );
	zephyrComp->SetEntityVariableInitializers( componentDef.GetZephyrEntityVarInits() );
	return zephyrComp;
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::InitializeZephyrEntityVariables( ZephyrComponent* zephyrComp )
{
	if ( zephyrComp == nullptr || !zephyrComp->IsScriptValid() )
	{
		return;
	}

	zephyrComp->InitializeEntityVariables();
}


//-----------------------------------------------------------------------------------------------
// Used for debug printing
const ZephyrBytecodeChunk* ZephyrSystem::GetBytecodeChunkByName( ZephyrComponent* zephyrComp, const std::string& chunkName )
{
	if ( zephyrComp == nullptr )
	{
		return nullptr;
	}

	return zephyrComp->GetBytecodeChunkByName( chunkName );
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::InitializeGlobalVariables( ZephyrComponent* zephyrComp, const ZephyrValueMap& initialValues )
{
	if ( zephyrComp->m_globalBytecodeChunk == nullptr )
	{
		return;
	}

	ZephyrValueMap* globalVariables = zephyrComp->m_globalBytecodeChunk->GetUpdateableVariables();
	if ( globalVariables == nullptr )
	{
		return;
	}

	for ( auto const& initialValue : initialValues )
	{
		const auto globalVarIter = globalVariables->find( initialValue.first );
		if ( globalVarIter == globalVariables->end() )
		{
			g_devConsole->PrintError( Stringf( "Cannot initialize nonexistent variable '%s' in script '%s'", initialValue.first.c_str(), zephyrComp->m_name.c_str() ) );
			zephyrComp->m_state = eComponentState::INVALID_SCRIPT;
			continue;
		}

		( *globalVariables )[initialValue.first] = initialValue.second;
	}
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrSystem::GetGlobalVariable( ZephyrComponent* zephyrComp, const std::string& varName )
{
	if ( zephyrComp == nullptr || !zephyrComp->IsScriptValid() )
	{
		return ZephyrValue( ERROR_ZEPHYR_VAL );
	}

	return zephyrComp->GetGlobalVariable( varName );
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::SetGlobalVariable( ZephyrComponent* zephyrComp, const std::string& varName, const ZephyrValue& value )
{
	if ( zephyrComp == nullptr || !zephyrComp->IsScriptValid() )
	{
		return;
	}

	zephyrComp->SetGlobalVariable( varName, value );
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::ChangeZephyrScriptState( ZephyrComponent* zephyrComp, const std::string& targetState )
{
	if ( zephyrComp == nullptr )
	{
		return;
	}

	if ( !zephyrComp->IsScriptValid() )
	{
		g_devConsole->PrintWarning( Stringf( "Tried to change state of ZephyrComponent: %s to %s, but it doesn't have a valid script", zephyrComp->m_name.c_str(), targetState.c_str() ) );
		return;
	}

	ZephyrBytecodeChunk* targetStateBytecodeChunk = zephyrComp->GetStateBytecodeChunk( targetState );
	if ( targetStateBytecodeChunk == nullptr
		 || targetStateBytecodeChunk == zephyrComp->m_curStateBytecodeChunk )
	{
		// State doesn't exist, should be reported by compiler to avoid flooding with errors here
		// Or the state change is a no-op
		return;
	}

	ZephyrSystem::FireScriptEvent( zephyrComp, "OnExit" );

	zephyrComp->m_curStateBytecodeChunk = targetStateBytecodeChunk;
	// Initialize state variables each time the state is entered
	ZephyrInterpreter::InterpretStateBytecodeChunk( *zephyrComp->m_curStateBytecodeChunk, zephyrComp->m_globalBytecodeChunk->GetUpdateableVariables(), *zephyrComp, zephyrComp->m_curStateBytecodeChunk->GetUpdateableVariables() );

	ZephyrSystem::FireScriptEvent( zephyrComp, "OnEnter" );
	zephyrComp->m_hasEnteredStartingState = true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::UnloadZephyrScript( ZephyrComponent* zephyrComp )
{
	if ( zephyrComp == nullptr || !zephyrComp->IsScriptValid() )
	{
		return;
	}

	zephyrComp->m_stateBytecodeChunks.clear();
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::ReloadZephyrScript( ZephyrComponent* zephyrComp )
{
	if ( zephyrComp == nullptr )
	{
		return;
	}

	zephyrComp->Destroy();

	if ( !zephyrComp->Initialize() )
	{
		return;
	}

	zephyrComp->InterpretGlobalBytecodeChunk();
	ZephyrSystem::InitializeGlobalVariables( zephyrComp, zephyrComp->m_componentDef.GetZephyrScriptInitialValues() );
	zephyrComp->SetEntityVariableInitializers( zephyrComp->m_componentDef.GetZephyrEntityVarInits() );
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::UpdateScene( ZephyrScene& scene )
{
	for ( ZephyrComponent*& comp : scene.zephyrComponents )
	{
		if ( comp == nullptr )
		{
			continue;
		}

		if ( !comp->IsScriptValid() )
		{
			EventArgs args;
			args.SetValue( "entity", (void*)comp->GetParentEntity() );
			args.SetValue( "text", "Script Error" );
			args.SetValue( "color", "red" );

			g_eventSystem->FireEvent( "PrintDebugText", &args );
			continue;
		}

		// If this is the first update we need to call OnEnter explicitly
		if ( !comp->m_hasEnteredStartingState )
		{
			comp->m_hasEnteredStartingState = true;

			ZephyrSystem::FireScriptEvent( comp, "OnEnter" );
		}
		
		ZephyrSystem::FireScriptEvent( comp, "OnUpdate" );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::FireSpawnEvent( ZephyrComponent* zephyrComp )
{
	if ( zephyrComp == nullptr || zephyrComp->m_state != eComponentState::INITIALIZED )
	{
		return;
	}

	EventArgs args;
	args.SetValue( "EntityId", zephyrComp->m_parentEntity->GetId() );
	args.SetValue( "EntityName", zephyrComp->m_parentEntity->GetName() );

	ZephyrSystem::FireScriptEvent( zephyrComp, "OnSpawn", &args );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrSystem::FireScriptEvent( ZephyrComponent* zephyrComp, const std::string& eventName, EventArgs* args )
{
	if ( zephyrComp == nullptr || zephyrComp->m_state != eComponentState::INITIALIZED )
	{
		return false;
	}

	EventArgs eventArgs;
	if ( args == nullptr )
	{
		args = &eventArgs;
	}

	ZephyrBytecodeChunk* eventChunk = zephyrComp->GetEventBytecodeChunk( eventName );
	if ( eventChunk == nullptr )
	{
		return false;
	}

	ZephyrValueMap* stateVariables = nullptr;
	if ( zephyrComp->m_curStateBytecodeChunk != nullptr )
	{
		stateVariables = zephyrComp->m_curStateBytecodeChunk->GetUpdateableVariables();
	}

	//zephyrComp->m_parentEntity->AddGameEventParams( args );

	ZephyrInterpreter::InterpretEventBytecodeChunk( *eventChunk, zephyrComp->m_globalBytecodeChunk->GetUpdateableVariables(), *zephyrComp, args, stateVariables );
	return true;
}


