#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"
#include "Engine/Zephyr/Core/ZephyrBytecodeChunk.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponent.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrEntity.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrEntityDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrComponent* ZephyrSystem::CreateComponent( ZephyrEntity* parentEntity, const ZephyrEntityDefinition& entityDef )
{
	ZephyrScriptDefinition* scriptDef = entityDef.GetZephyrScriptDefinition();
	if ( scriptDef == nullptr )
	{
		return nullptr;
	}
	
	ZephyrComponent* zephyrComp = new ZephyrComponent( *scriptDef, parentEntity );
	parentEntity->SetZephyrComponent( zephyrComp );
	zephyrComp->InterpretGlobalBytecodeChunk();
	ZephyrSystem::InitializeGlobalVariables( zephyrComp, entityDef.GetZephyrScriptInitialValues() );
	zephyrComp->SetEntityVariableInitializers( entityDef.GetZephyrEntityVarInits() );
	return zephyrComp;
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::InitializeZephyrEntityVariables()
{

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
			zephyrComp->m_isScriptObjectValid = false;
			continue;
		}

		( *globalVariables )[initialValue.first] = initialValue.second;
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::UpdateComponents( std::vector<ZephyrComponent*>& components )
{
	for ( ZephyrComponent*& comp : components )
	{
		if ( !comp->IsScriptValid() )
		{
			EventArgs args;
			args.SetValue( "entity", (void*)comp->GetParentEntity() );
			args.SetValue( "text", "Script Error" );
			args.SetValue( "color", "red" );

			g_eventSystem->FireEvent( "PrintDebugText", &args );
			return;
		}

		// If this is the first update we need to call OnEnter explicitly
		/*if ( !m_hasEnteredStartingState )
		{
			m_hasEnteredStartingState = true;

			FireEvent( "OnEnter" );
		}*/

		comp->FireEvent( "OnUpdate" );
	}
}


