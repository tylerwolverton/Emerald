#include "Engine/Physics/3D/PhysicsSystem3D.hpp"
#include "Engine/Physics/3D/Rigidbody3D.hpp"
#include "Engine/Time/Clock.hpp"


//-----------------------------------------------------------------------------------------------
void PhysicsSystem3D::Startup( Clock* gameClock )
{
	m_gameClock = gameClock;
	if ( m_gameClock == nullptr )
	{
		m_gameClock = Clock::GetMaster();
	}

	m_physicsClock = new Clock( m_gameClock );
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem3D::Update()
{
	AdvanceSimulation( m_physicsClock->GetLastDeltaSeconds() );
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem3D::Shutdown()
{
	DestroyAllRigidbodies();
	DestroyAllColliders();
	CleanupDestroyedObjects();

	PTR_SAFE_DELETE( m_physicsClock );
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem3D::Reset()
{
	DestroyAllRigidbodies();
	DestroyAllColliders();
	CleanupDestroyedObjects();
}


//-----------------------------------------------------------------------------------------------
Rigidbody3D* PhysicsSystem3D::CreateRigidbody()
{
	Rigidbody3D* newRigidbody = new Rigidbody3D();
	newRigidbody->m_system = this;

	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		if ( m_rigidbodies[rigidbodyIdx] == nullptr )
		{
			m_rigidbodies[rigidbodyIdx] = newRigidbody;
			return newRigidbody;
		}
	}

	m_rigidbodies.push_back( newRigidbody );

	return newRigidbody;
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem3D::DestroyRigidbody( Rigidbody3D* rigidbodyToDestroy )
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		if ( m_rigidbodies[rigidbodyIdx] == rigidbodyToDestroy )
		{
			m_garbageRigidbodyIndexes.push_back( rigidbodyIdx );
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem3D::AdvanceSimulation( float deltaSeconds )
{
	ApplyEffectors(); 					// apply gravity to all dynamic objects
	MoveRigidbodies( deltaSeconds ); 	// apply an euler step to all rigidbodies, and reset per-frame data
	//DetectCollisions();					// determine all pairs of intersecting colliders
	//ClearOldCollisions();
	//ResolveCollisions(); 				// resolve all collisions, firing appropraite events
	//CleanupDestroyedObjects();  		// destroy objects 

	++m_frameNum;
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem3D::ApplyEffectors()
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		Rigidbody3D*& rigidbody = m_rigidbodies[rigidbodyIdx];
		if ( rigidbody != nullptr )
		{
			//switch ( rigidbody->GetSimulationMode() )
			//{
			//	case SIMULATION_MODE_DYNAMIC:
			//	{
					rigidbody->AddForce( m_forceOfGravity );
					rigidbody->ApplyDragForce();
			//	}
			//	break;
			//}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem3D::MoveRigidbodies( float deltaSeconds )
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		Rigidbody3D*& rigidbody = m_rigidbodies[rigidbodyIdx];
		if ( rigidbody != nullptr )
		{
			rigidbody->Update( deltaSeconds );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem3D::DestroyAllRigidbodies()
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		m_garbageRigidbodyIndexes.push_back( rigidbodyIdx );
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem3D::DestroyAllColliders()
{
	for ( int colliderIdx = 0; colliderIdx < (int)m_colliders.size(); ++colliderIdx )
	{
		m_garbageColliderIndexes.push_back( colliderIdx );
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem3D::CleanupDestroyedObjects()
{
	// Cleanup rigidbodies
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_garbageRigidbodyIndexes.size(); ++rigidbodyIdx )
	{
		Rigidbody3D*& garbageRigidbody = m_rigidbodies[m_garbageRigidbodyIndexes[rigidbodyIdx]];
		PTR_SAFE_DELETE( garbageRigidbody );
	}

	m_garbageRigidbodyIndexes.clear();

	// Cleanup colliders
	for ( int colliderIdx = 0; colliderIdx < (int)m_garbageColliderIndexes.size(); ++colliderIdx )
	{
		Collider3D*& garbageCollider = m_colliders[m_garbageColliderIndexes[colliderIdx]];
		PTR_SAFE_DELETE( garbageCollider );
	}

	m_garbageColliderIndexes.clear();
}


