#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/CollisionResolver.hpp"
#include "Engine/Physics/Manifold.hpp"
#include "Engine/Physics/PhysicsScene.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Timer.hpp"


//-----------------------------------------------------------------------------------------------
static float s_fixedDeltaSeconds = 1.0f / 120.0f;


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::Startup( Clock* gameClock )
{
	m_gameClock = gameClock;
	if ( m_gameClock == nullptr )
	{
		m_gameClock = Clock::GetMaster();
	}

	m_physicsClock = new Clock( m_gameClock );
	m_stepTimer = new Timer( m_physicsClock );

	m_stepTimer->SetSeconds( s_fixedDeltaSeconds );

	g_eventSystem->RegisterEvent( "set_physics_update", "Usage: set_physics_update hz=NUMBER .Set rate of physics update in hz.", eUsageLocation::DEV_CONSOLE, SetPhysicsUpdateRate );
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::Update( PhysicsScene& scene )
{
	//while ( m_stepTimer->CheckAndDecrement() )
	//{
	//	AdvanceSimulation( scene, s_fixedDeltaSeconds );
	//}
		
	AdvanceSimulation( scene, m_physicsClock->GetLastDeltaSeconds() );
	scene.DebugRender();
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::AdvanceSimulation( PhysicsScene& scene, float deltaSeconds )
{
	scene.ApplyAffectors(); 															// apply gravity (or other scene wide effects) to all dynamic objects
	MoveRigidbodies( scene.m_rigidbodies, deltaSeconds ); 								// apply an euler step to all rigidbodies, and reset per-frame data
	if ( scene.m_collisionResolver != nullptr )
	{
		scene.m_collisionResolver->ResolveCollisions( scene.m_colliders, m_frameNum ); 	// resolve all collisions, firing appropriate events, TODO: Move to this class?
	}
	scene.CleanupDestroyedObjects();  													// destroy objects 

	++m_frameNum;
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::MoveRigidbodies( std::vector<Rigidbody*>& rigidbodies, float deltaSeconds )
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
void PhysicsSystem::Shutdown()
{
	PTR_SAFE_DELETE( m_stepTimer );
	PTR_SAFE_DELETE( m_physicsClock );
}


//-----------------------------------------------------------------------------------------------
float PhysicsSystem::GetFixedDeltaSeconds() const
{
	return s_fixedDeltaSeconds;
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::SetFixedDeltaSeconds( float newDeltaSeconds )
{
	s_fixedDeltaSeconds = newDeltaSeconds;
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::ResetFixedDeltaSecondsToDefault()
{
	SetFixedDeltaSeconds( 1.f / 120.f );
}


//-----------------------------------------------------------------------------------------------
bool PhysicsSystem::SetPhysicsUpdateRate( EventArgs* args )
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
