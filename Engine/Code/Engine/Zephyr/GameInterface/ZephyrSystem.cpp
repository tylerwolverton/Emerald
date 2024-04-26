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
	if ( parentEntity == nullptr )
	{
		g_devConsole->PrintError("Can't create a zephyr component with a null parent entity");
		return nullptr;
	}

	ZephyrComponent* zephyrComp = new ZephyrComponent( parentEntity->GetId(), componentDef );
	if ( !zephyrComp->Initialize() )
	{
		return zephyrComp;
	}

	zephyrComp->InterpretGlobalBytecodeChunk();
	ZephyrSystem::InitializeGlobalVariables( zephyrComp, componentDef.zephyrScriptInitialValues );
	zephyrComp->SetEntityVariableInitializers( componentDef.zephyrEntityVarInits );
	return zephyrComp;
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::InitializeAllZephyrEntityVariables( ZephyrScene& scene )
{
	for ( ZephyrComponent*& comp : scene.zephyrComponents )
	{
		InitializeZephyrEntityVariables( comp );
	}
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
void ZephyrSystem::InitializeZephyrEntityVariables( const EntityId& entityId )
{
	ZephyrComponent* zephyrComp = (ZephyrComponent*)GetComponentFromEntityId( entityId, ENTITY_COMPONENT_TYPE_ZEPHYR );

	return InitializeZephyrEntityVariables( zephyrComp );
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
const ZephyrBytecodeChunk* ZephyrSystem::GetBytecodeChunkByName( const EntityId& entityId, const std::string& chunkName )
{
	ZephyrComponent* zephyrComp = (ZephyrComponent*)GetComponentFromEntityId( entityId, ENTITY_COMPONENT_TYPE_ZEPHYR );

	return GetBytecodeChunkByName( zephyrComp, chunkName );
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::InitializeGlobalVariables( ZephyrComponent* zephyrComp, const ZephyrValueMap& initialValues )
{
	if ( zephyrComp == nullptr || zephyrComp->m_globalBytecodeChunk == nullptr )
	{
		return;
	}

	ZephyrScope* globalVariableScope = zephyrComp->m_globalBytecodeChunk->GetVariableScope();
	if ( globalVariableScope == nullptr )
	{
		return;
	}

	for ( auto const& initialValue : initialValues )
	{
		if ( !globalVariableScope->SetVariable( initialValue.first.c_str(), initialValue.second ) )
		{
			g_devConsole->PrintError( Stringf( "Cannot initialize nonexistent variable '%s' in script '%s'", initialValue.first.c_str(), zephyrComp->GetScriptName().c_str() ) );
			zephyrComp->m_compState = eComponentState::INVALID_SCRIPT;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::InitializeGlobalVariables( const EntityId& entityId, const ZephyrValueMap& initialValues )
{
	ZephyrComponent* zephyrComp = (ZephyrComponent*)GetComponentFromEntityId( entityId, ENTITY_COMPONENT_TYPE_ZEPHYR );

	return InitializeGlobalVariables( zephyrComp, initialValues );
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrSystem::GetGlobalVariable( ZephyrComponent* zephyrComp, const std::string& varName )
{
	if ( zephyrComp == nullptr || !zephyrComp->IsScriptValid() )
	{
		return ZephyrValue::ERROR_VALUE;
	}

	// Try to get native first
	EventArgs args;
	args.SetValue( PARENT_ENTITY_ID_STR, zephyrComp->GetParentEntityId() );
	args.SetValue( "varName", varName );

	// TODO: Native variables must be a ZephyrType
	//g_eventSystem->FireEvent( "GetNativeEntityVariable", &args );

	// If this wasn't native it must be a script variable
	bool isNative = args.GetValue( "isNative", false );
	if ( isNative )
	{
		return args.GetValue( "zephyrValue", ZephyrValue() );
	}

	return zephyrComp->GetGlobalVariable( varName );
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrSystem::GetGlobalVariable( const EntityId& entityId, const std::string& varName )
{
	ZephyrComponent* zephyrComp = (ZephyrComponent*)GetComponentFromEntityId( entityId, ENTITY_COMPONENT_TYPE_ZEPHYR );
	
	return GetGlobalVariable( zephyrComp, varName );
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::SetGlobalVariable( ZephyrComponent* zephyrComp, const std::string& varName, const ZephyrValue& value )
{
	if ( zephyrComp == nullptr || !zephyrComp->IsScriptValid() )
	{
		return;
	}

	// Try to set native first
	EventArgs args;
	args.SetValue( PARENT_ENTITY_ID_STR, zephyrComp->GetParentEntityId() );
	args.SetValue( "varName", varName );
	args.SetValue( "zephyrValue", value );
	g_eventSystem->FireEvent( "SetNativeEntityVariable", &args );

	// If this wasn't native it must be a script variable
	bool isNative = args.GetValue( "isNative", false );
	if ( !isNative )
	{
		zephyrComp->SetGlobalVariable( varName, value );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::SetGlobalVariable( const EntityId& entityId, const std::string& varName, const ZephyrValue& value )
{
	ZephyrComponent* zephyrComp = (ZephyrComponent*)GetComponentFromEntityId( entityId, ENTITY_COMPONENT_TYPE_ZEPHYR );

	return SetGlobalVariable( zephyrComp, varName, value );
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
		g_devConsole->PrintWarning( Stringf( "Tried to change state of ZephyrComponent: %s to %s, but it doesn't have a valid script", zephyrComp->GetScriptName().c_str(), targetState.c_str() ) );
		return;
	}

	if ( targetState.empty() )
	{
		g_devConsole->PrintWarning( Stringf( "Tried to change state of ZephyrComponent: %s to empty state", zephyrComp->GetScriptName().c_str() ) );
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
	ZephyrInterpreter::InterpretStateBytecodeChunk( *zephyrComp->m_curStateBytecodeChunk, *zephyrComp );

	ZephyrSystem::FireScriptEvent( zephyrComp, "OnEnter" );
	//zephyrComp->m_state = eComponentState::STARTED;
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::ChangeZephyrScriptState( const EntityId& entityId, const std::string& targetState )
{
	ZephyrComponent* zephyrComp = (ZephyrComponent*)GetComponentFromEntityId( entityId, ENTITY_COMPONENT_TYPE_ZEPHYR );

	return ChangeZephyrScriptState( zephyrComp, targetState );
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::UnloadZephyrScripts( ZephyrScene& scene )
{
	for ( ZephyrComponent*& comp : scene.zephyrComponents )
	{
		UnloadZephyrScript( comp );
	}
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
void ZephyrSystem::ReloadZephyrScripts( ZephyrScene& scene )
{
	for ( ZephyrComponent*& comp : scene.zephyrComponents )
	{
		ReloadZephyrScript( comp );
	}
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
	ZephyrSystem::InitializeGlobalVariables( zephyrComp, zephyrComp->m_componentDef.zephyrScriptInitialValues );
	zephyrComp->SetEntityVariableInitializers( zephyrComp->m_componentDef.zephyrEntityVarInits );
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::UpdateScene( ZephyrScene& scene )
{
	for ( ZephyrComponent*& zephyrComp : scene.zephyrComponents )
	{
		if ( zephyrComp == nullptr )
		{
			continue;
		}

		UpdateComponent( zephyrComp );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::FireAllSpawnEvents( ZephyrScene& scene )
{
	for ( ZephyrComponent*& zephyrComp : scene.zephyrComponents )
	{
		if ( zephyrComp == nullptr )
		{
			continue;
		}

		FireSpawnEvent( zephyrComp );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::UpdateComponent( ZephyrComponent* zephyrComp )
{
	if ( !zephyrComp->IsScriptValid() )
	{
		// TODO: Make sure this still works after type upgrade
		EventArgs args;
		args.SetValue( PARENT_ENTITY_ID_STR, zephyrComp->GetParentEntityId() );
		args.SetValue( "text", "Script Error" );
		args.SetValue( "color", "red" );

		g_eventSystem->FireEvent( "PrintDebugText", &args );
		return;
	}

	// If this is the first update we need to call OnEnter explicitly
	if ( zephyrComp->m_compState == eComponentState::INITIALIZED )
	{
		zephyrComp->m_compState = eComponentState::STARTED;

		ZephyrSystem::FireScriptEvent( zephyrComp, "OnEnter" );
	}

	ZephyrSystem::FireScriptEvent( zephyrComp, "OnUpdate" );
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::FireSpawnEvent( ZephyrComponent* zephyrComp )
{
	if ( zephyrComp == nullptr || !zephyrComp->IsScriptValid() )
	{
		return;
	}

	EventArgs args;
	args.SetValue( PARENT_ENTITY_ID_STR, zephyrComp->GetParentEntityId() );
	args.SetValue( PARENT_ENTITY_NAME_STR, zephyrComp->GetParentEntityName() );

	ZephyrSystem::FireScriptEvent( zephyrComp, "OnSpawn", &args );
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::FireSpawnEvent( const EntityId& entityId )
{
	ZephyrComponent* zephyrComp = (ZephyrComponent*)GetComponentFromEntityId( entityId, ENTITY_COMPONENT_TYPE_ZEPHYR );

	return FireSpawnEvent( zephyrComp );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrSystem::FireScriptEvent( ZephyrComponent* zephyrComp, const std::string& eventName, EventArgs* args )
{
	if ( zephyrComp == nullptr || !zephyrComp->IsScriptValid() )
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

	//zephyrComp->m_parentEntity->AddGameEventParams( args );

	ZephyrInterpreter::InterpretEventBytecodeChunk( *eventChunk, *zephyrComp, args );
	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrSystem::FireScriptEvent( const EntityId& entityId, const std::string& eventName, EventArgs* args )
{
	ZephyrComponent* zephyrComp = (ZephyrComponent*)GetComponentFromEntityId( entityId, ENTITY_COMPONENT_TYPE_ZEPHYR );

	return FireScriptEvent( zephyrComp, eventName, args );
}


