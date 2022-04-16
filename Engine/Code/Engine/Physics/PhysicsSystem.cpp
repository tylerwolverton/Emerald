#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/Collision.hpp"
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
	while ( m_stepTimer->CheckAndDecrement() )
	{
		AdvanceSimulation( scene, s_fixedDeltaSeconds );
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::AdvanceSimulation( PhysicsScene& scene, float deltaSeconds )
{
	ApplyEffectors(); 										// apply gravity (or other scene wide effects) to all dynamic objects
	MoveRigidbodies( scene.rigidbodies, deltaSeconds ); 	// apply an euler step to all rigidbodies, and reset per-frame data
	if ( scene.collisionResolver != nullptr )
	{
		scene.collisionResolver->ResolveCollisions( scene.colliders, m_frameNum ); 			// resolve all collisions, firing appropraite events, TODO: Move to this class?
	}
	CleanupDestroyedObjects();  		// destroy objects 

	++m_frameNum;
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::MoveRigidbodies( std::vector<Rigidbody>& rigidbodies, float deltaSeconds )
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)rigidbodies.size(); ++rigidbodyIdx )
	{
		Rigidbody& rigidbody = rigidbodies[rigidbodyIdx];
		switch ( rigidbody.GetSimulationMode() )
		{
			case SIMULATION_MODE_DYNAMIC:
			case SIMULATION_MODE_KINEMATIC:
			{
				rigidbody.Update( deltaSeconds );
			}
		}
	}
}


//
////-----------------------------------------------------------------------------------------------
//void PhysicsSystem::DestroyAllRigidbodies()
//{
//	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
//	{
//		m_garbageRigidbodyIndexes.push_back( rigidbodyIdx );		
//	}
//}
//
//
////-----------------------------------------------------------------------------------------------
//void PhysicsSystem::DestroyAllColliders()
//{
//	for ( int colliderIdx = 0; colliderIdx < (int)m_colliders.size(); ++colliderIdx )
//	{
//		m_garbageColliderIndexes.push_back( colliderIdx );
//	}
//}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::CleanupDestroyedObjects()
{
	// Cleanup rigidbodies
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_garbageRigidbodyIndexes.size(); ++rigidbodyIdx )
	{
		Rigidbody*& garbageRigidbody = m_rigidbodies[m_garbageRigidbodyIndexes[rigidbodyIdx]];
		PTR_SAFE_DELETE( garbageRigidbody );
	}

	m_garbageRigidbodyIndexes.clear();

	// Cleanup colliders
	for ( int colliderIdx = 0; colliderIdx < (int)m_garbageColliderIndexes.size(); ++colliderIdx )
	{
		Collider*& garbageCollider = m_colliders[m_garbageColliderIndexes[colliderIdx]];
		PTR_SAFE_DELETE( garbageCollider );
	}

	m_garbageColliderIndexes.clear();
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::Shutdown()
{
	PTR_SAFE_DELETE( m_stepTimer );
	PTR_SAFE_DELETE( m_physicsClock );
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::Reset()
{
	ClearOldCollisions();

	DestroyAllColliders();
	DestroyAllRigidbodies();
	CleanupDestroyedObjects();

	m_colliders.clear();
	m_rigidbodies.clear();
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
