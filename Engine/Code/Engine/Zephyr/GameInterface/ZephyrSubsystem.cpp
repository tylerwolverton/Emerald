#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Engine/Zephyr/Core/ZephyrUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrEngineEvents.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"
#include "Engine/Zephyr/Types/ZephyrBool.hpp"
#include "Engine/Zephyr/Types/ZephyrNumber.hpp"
#include "Engine/Zephyr/Types/ZephyrString.hpp"
#include "Engine/Zephyr/Types/ZephyrVec2.hpp"
#include "Engine/Zephyr/Types/ZephyrVec3.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Framework/Entity.hpp"
#include "Engine/Time/Clock.hpp"


//-----------------------------------------------------------------------------------------------
void ZephyrSubsystem::Startup( const ZephyrSystemParams& params )
{
	if ( params.clock == nullptr )
	{
		m_clock = Clock::GetMaster();
	}
	else
	{
		m_clock = params.clock;
	}

	constexpr int POOL_SIZE = 50;
	m_timerPool.reserve( POOL_SIZE );
	for ( int timerIdx = 0; timerIdx < POOL_SIZE; ++timerIdx )
	{
		m_timerPool.emplace_back( m_clock );
	}

	// Register engine types
	ZephyrNumber::CreateAndRegisterMetadata();
	ZephyrString::CreateAndRegisterMetadata();
	ZephyrBool::CreateAndRegisterMetadata();
	ZephyrVec2::CreateAndRegisterMetadata();
	ZephyrVec3::CreateAndRegisterMetadata();
}


//-----------------------------------------------------------------------------------------------
void ZephyrSubsystem::Update()
{
	UpdateTimers();
}


//-----------------------------------------------------------------------------------------------
void ZephyrSubsystem::UpdateTimers()
{
	int numTimers = (int)m_timerPool.size();
	for ( int timerIdx = 0; timerIdx < numTimers; ++timerIdx )
	{
		ZephyrTimer& zephyrTimer = m_timerPool[timerIdx];
		if ( !zephyrTimer.timer.IsRunning() 
			 || !zephyrTimer.timer.HasElapsed() )
		{
			continue;
		}

		if ( !zephyrTimer.callbackName.empty() )
		{
			if ( zephyrTimer.targetId == INVALID_ENTITY_ID )
			{
				g_eventSystem->FireEvent( zephyrTimer.callbackName, zephyrTimer.callbackArgs );
			}
			else
			{
				Entity* targetEntity = g_zephyrAPI->GetEntityById( zephyrTimer.targetId );
				if ( targetEntity != nullptr )
				{
					ZephyrSystem::FireScriptEvent( zephyrTimer.targetId, zephyrTimer.callbackName, zephyrTimer.callbackArgs );
				}
			}
		}

		zephyrTimer.timer.Stop();
		zephyrTimer.callbackArgs->Reset();
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrSubsystem::Shutdown()
{
	m_timerPool.clear();
	ResetRegisteredUserTypes();
}


//-----------------------------------------------------------------------------------------------
void ZephyrSubsystem::StartNewTimer( const EntityId& targetId, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs )
{
	int numTimers = (int)m_timerPool.size();
	for ( int timerIdx = 0; timerIdx < numTimers; ++timerIdx )
	{
		if ( !m_timerPool[timerIdx].timer.IsRunning() )
		{
			ZephyrTimer& freeZephyrTimer = m_timerPool[timerIdx];
			freeZephyrTimer.targetId = targetId;
			freeZephyrTimer.callbackName = onCompletedEventName;
			freeZephyrTimer.name = name;
			CloneZephyrEventArgs( freeZephyrTimer.callbackArgs, *callbackArgs );

			freeZephyrTimer.timer.Start( (double)durationSeconds );
			return;
		}
	}

	g_devConsole->PrintError( "No more room for new ZephyrTimer" );
}


//-----------------------------------------------------------------------------------------------
void ZephyrSubsystem::StartNewTimer( const std::string& targetName, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs )
{
	Entity* target = g_zephyrAPI->GetEntityByName( targetName );

	if ( target == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Couldn't start a timer event with unknown target name '%s'", targetName.c_str() ) );
		return;
	}

	StartNewTimer( target->GetId(), name, durationSeconds, onCompletedEventName, callbackArgs );
}


//-----------------------------------------------------------------------------------------------
void ZephyrSubsystem::StopAllTimers()
{
	int numTimers = (int)m_timerPool.size();
	for ( int timerIdx = 0; timerIdx < numTimers; ++timerIdx )
	{
		m_timerPool[timerIdx].timer.Stop();
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrSubsystem::RegisterZephyrType( ZephyrTypeMetadata* typeMetadata )
{
	GUARANTEE_OR_DIE( typeMetadata != nullptr, "Can't register a null ZephyrTypeMetadata" );

	const auto& iter = m_registeredZephyrTypes.find( typeMetadata->GetTypeName() );
	if ( iter != m_registeredZephyrTypes.cend() )
	{
		return;
	}

	m_registeredZephyrTypes[typeMetadata->GetTypeName()] = typeMetadata;
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeMetadata* ZephyrSubsystem::GetRegisteredUserType( const std::string& typeName ) const
{
	const auto& iter = m_registeredZephyrTypes.find( typeName );
	if ( iter != m_registeredZephyrTypes.cend() )
	{
		return iter->second;
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeMetadata ZephyrSubsystem::GetCopyOfRegisteredUserType( const std::string& typeName ) const
{
	const auto& iter = m_registeredZephyrTypes.find( typeName );
	if ( iter != m_registeredZephyrTypes.cend() )
	{
		return *iter->second;
	}

	return ZephyrTypeMetadata();
}


//-----------------------------------------------------------------------------------------------
void ZephyrSubsystem::ResetRegisteredUserTypes()
{
	PTR_MAP_SAFE_DELETE( m_registeredZephyrTypes );
}
