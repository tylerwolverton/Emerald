#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Engine/Zephyr/Core/ZephyrUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrEngineEvents.hpp"
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
			if ( zephyrTimer.targetId == -1 )
			{
				g_eventSystem->FireEvent( zephyrTimer.callbackName, zephyrTimer.callbackArgs );
			}
			else
			{
				Entity* targetEntity = g_zephyrAPI->GetEntityById( zephyrTimer.targetId );
				if ( targetEntity != nullptr )
				{
					// TODO: Add direct access to components from id
					//targetEntity->FireScriptEvent( zephyrTimer.callbackName, zephyrTimer.callbackArgs );
				}
			}
		}

		zephyrTimer.timer.Stop();
		zephyrTimer.callbackArgs->Clear();
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrSubsystem::Shutdown()
{
	m_timerPool.clear();
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
