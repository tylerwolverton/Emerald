#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/3D/OBB3Collider.hpp"
#include "Engine/Physics/3D/SphereCollider.hpp"
#include "Engine/Physics/CollisionResolvers/CollisionPolicies.hpp"
#include "Engine/Physics/Manifold.hpp"
#include "Engine/Physics/PhysicsScene.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Timer.hpp"


//-----------------------------------------------------------------------------------------------
static float s_fixedDeltaSeconds = 1.0f / 120.0f;


//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
void PhysicsSystem<CollisionPolicy>::Startup( Clock* gameClock )
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

	g_colliderFactory->RegisterCreator( "obb3", &OBB3Collider::Create );
	g_colliderFactory->RegisterCreator( "sphere", &SphereCollider::Create );
}


//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
void PhysicsSystem<CollisionPolicy>::Update( PhysicsScene& scene )
{
	//while ( m_stepTimer->CheckAndDecrement() )
	//{
	//	AdvanceSimulation( scene, s_fixedDeltaSeconds );
	//}

	AdvanceSimulation( scene, (float)m_physicsClock->GetLastDeltaSeconds() );
}


//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
void PhysicsSystem<CollisionPolicy>::AdvanceSimulation( PhysicsScene& scene, float deltaSeconds )
{
	ApplyAffectors( scene.rigidbodies, scene.affectors ); 									// apply gravity (or other scene wide effects) to all dynamic objects
	MoveRigidbodies( scene.rigidbodies, deltaSeconds ); 									// apply an euler step to all rigidbodies, and reset per-frame data
	CollisionPolicy::ResolveCollisions( scene.colliders, scene.collisions, m_frameNum );	// resolve all collisions, firing appropriate events
	scene.CleanupDestroyedObjects();  														// destroy objects 

	++m_frameNum;
}


//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
void PhysicsSystem<CollisionPolicy>::ApplyAffectors( RigidbodyVector& rigidbodies, const AffectorVector& affectors )
{
	for ( auto& rigidbody : rigidbodies )
	{
		if ( rigidbody->GetSimulationMode() == SIMULATION_MODE_DYNAMIC )
		{
			for ( const auto& affector : affectors )
			{
				affector( rigidbody );
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
void PhysicsSystem<CollisionPolicy>::MoveRigidbodies( RigidbodyVector& rigidbodies, float deltaSeconds )
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
template <class CollisionPolicy>
void PhysicsSystem<CollisionPolicy>::Shutdown()
{
	PTR_SAFE_DELETE( m_stepTimer );
	PTR_SAFE_DELETE( m_physicsClock );
}


//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
float PhysicsSystem<CollisionPolicy>::GetFixedDeltaSeconds() const
{
	return s_fixedDeltaSeconds;
}


//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
void PhysicsSystem<CollisionPolicy>::SetFixedDeltaSeconds( float newDeltaSeconds )
{
	s_fixedDeltaSeconds = newDeltaSeconds;
}


//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
void PhysicsSystem<CollisionPolicy>::ResetFixedDeltaSecondsToDefault()
{
	SetFixedDeltaSeconds( 1.f / 120.f );
}


//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
bool PhysicsSystem<CollisionPolicy>::SetPhysicsUpdateRate( EventArgs* args )
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
