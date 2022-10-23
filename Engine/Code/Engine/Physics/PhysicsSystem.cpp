#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Timer.hpp"


//-----------------------------------------------------------------------------------------------
static float s_fixedDeltaSeconds = 1.0f / 120.0f;


//-----------------------------------------------------------------------------------------------
void PhysicsSystemBase::Startup( Clock* gameClock )
{
	m_gameClock = gameClock;
	if ( m_gameClock == nullptr )
	{
		m_gameClock = Clock::GetMaster();
	}

	m_physicsClock = new Clock( m_gameClock );
	m_stepTimer = new Timer( m_physicsClock );

	m_stepTimer->SetSeconds( s_fixedDeltaSeconds );

	g_eventSystem->RegisterEvent( "set_physics_update", "Usage: set_physics_update hz=NUMBER. Set rate of physics update in hz.", eUsageLocation::DEV_CONSOLE, SetPhysicsUpdateRate );
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystemBase::Update( PhysicsScene& scene )
{
	while ( m_stepTimer->CheckAndDecrement() )
	{
		AdvanceSimulation( scene, s_fixedDeltaSeconds );
	}

	//AdvanceSimulation( scene, (float)m_physicsClock->GetLastDeltaSeconds() );

	// Update TransformComponents
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystemBase::ApplyAffectors( RigidbodyVector& rigidbodies, const AffectorMap& affectors )
{
	for ( auto& rigidbody : rigidbodies )
	{
		if ( rigidbody->GetSimulationMode() == SIMULATION_MODE_DYNAMIC )
		{
			for ( const auto& affector : affectors )
			{
				affector.second( rigidbody );
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystemBase::MoveRigidbodies( RigidbodyVector& rigidbodies, float deltaSeconds )
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)rigidbodies.size(); ++rigidbodyIdx )
	{
		Rigidbody*& rigidbody = rigidbodies[rigidbodyIdx];
		if ( rigidbody == nullptr )
		{
			continue;
		}

		switch ( rigidbody->GetSimulationMode() )
		{
			case SIMULATION_MODE_DYNAMIC:
			case SIMULATION_MODE_KINEMATIC:
			{
				rigidbody->Update( deltaSeconds );
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystemBase::Shutdown()
{
	PTR_SAFE_DELETE( m_stepTimer );
	PTR_SAFE_DELETE( m_physicsClock );
}


//-----------------------------------------------------------------------------------------------
float PhysicsSystemBase::GetFixedDeltaSeconds() const
{
	return s_fixedDeltaSeconds;
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystemBase::SetFixedDeltaSeconds( float newDeltaSeconds )
{
	s_fixedDeltaSeconds = newDeltaSeconds;
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystemBase::ResetFixedDeltaSecondsToDefault()
{
	SetFixedDeltaSeconds( 1.f / 120.f );
}


//-----------------------------------------------------------------------------------------------
bool PhysicsSystemBase::SetPhysicsUpdateRate( EventArgs* args )
{
	float hz = args->GetValue( "hz", 120.f );

	if ( hz == 0.f )
	{
		s_fixedDeltaSeconds = 0.f;
	}
	else
	{
		s_fixedDeltaSeconds = 1.f / hz;
	}

	return false;
}
