#include "Engine/ZephyrCore/ZephyrSystem.hpp"
#include "Engine/ZephyrCore/ZephyrCommon.hpp"
#include "Engine/ZephyrCore/ZephyrEngineAPI.hpp"
#include "Engine/ZephyrCore/ZephyrEntity.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Time/Clock.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrSystem::ZephyrSystem()
{

}


//-----------------------------------------------------------------------------------------------
ZephyrSystem::~ZephyrSystem()
{

}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::Startup( const ZephyrSystemParams& params )
{
	if ( params.clock == nullptr )
	{
		m_clock = Clock::GetMaster();
	}
	else
	{
		m_clock = params.clock;
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::Update()
{
	UpdateTimers();
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::UpdateTimers()
{
	int numTimers = (int)m_timerPool.size();
	for ( int timerIdx = 0; timerIdx < numTimers; ++timerIdx )
	{
		ZephyrTimer*& gameTimer = m_timerPool[timerIdx];
		if ( gameTimer == nullptr )
		{
			continue;
		}

		if ( gameTimer->timer.IsRunning()
			 && gameTimer->timer.HasElapsed() )
		{
			if ( !gameTimer->callbackName.empty() )
			{
				if ( gameTimer->targetId == -1 )
				{
					g_eventSystem->FireEvent( gameTimer->callbackName, gameTimer->callbackArgs );
				}
				else
				{
					ZephyrEntity* targetEntity = g_zephyrAPI->GetEntityById( gameTimer->targetId );
					if ( targetEntity != nullptr )
					{
						targetEntity->FireScriptEvent( gameTimer->callbackName, gameTimer->callbackArgs );
					}
				}
			}

			delete m_timerPool[timerIdx];
			m_timerPool[timerIdx] = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::Shutdown()
{
	PTR_VECTOR_SAFE_DELETE( m_timerPool );
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::StartNewTimer( const EntityId& targetId, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs )
{
	ZephyrTimer* newTimer = new ZephyrTimer( m_clock, targetId, onCompletedEventName, name, callbackArgs );

	int numTimers = (int)m_timerPool.size();
	for ( int timerIdx = 0; timerIdx < numTimers; ++timerIdx )
	{
		if ( m_timerPool[timerIdx] == nullptr )
		{
			m_timerPool[timerIdx] = newTimer;
			newTimer->timer.Start( (double)durationSeconds );
			return;
		}
	}

	newTimer->timer.Start( (double)durationSeconds );
	m_timerPool.push_back( newTimer );
}


//-----------------------------------------------------------------------------------------------
void ZephyrSystem::StartNewTimer( const std::string& targetName, const std::string& name, float durationSeconds, const std::string& onCompletedEventName, EventArgs* callbackArgs )
{
	ZephyrEntity* target = g_zephyrAPI->GetEntityByName( targetName );

	if ( target == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Couldn't start a timer event with unknown target name '%s'", targetName.c_str() ) );
		return;
	}

	StartNewTimer( target->GetId(), name, durationSeconds, onCompletedEventName, callbackArgs );
}
