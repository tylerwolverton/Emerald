#include "Engine/Physics/PhysicsScene.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Physics/CollisionResolver.hpp"


//-----------------------------------------------------------------------------------------------
PhysicsScene::~PhysicsScene()
{
	Reset();
	CleanupDestroyedObjects();
}


//-----------------------------------------------------------------------------------------------
Rigidbody* PhysicsScene::CreateCylinderRigidbody( const Vec3& worldPosition, float mass, float radius )
{
	Rigidbody* newRigidbody = new Rigidbody( mass );
	m_rigidbodies.push_back( newRigidbody );
	// Create sphere collider

	return newRigidbody;
}


//-----------------------------------------------------------------------------------------------
Rigidbody* PhysicsScene::CreateOBB3Rigidbody( const OBB3& box, float mass )
{
	Rigidbody* newRigidbody = new Rigidbody( mass );
	m_rigidbodies.push_back( newRigidbody );
	// Create sphere collider

	return newRigidbody;
}


//-----------------------------------------------------------------------------------------------
void PhysicsScene::Reset()
{
	DestroyAllColliders();
	DestroyAllRigidbodies();
}


//-----------------------------------------------------------------------------------------------
void PhysicsScene::ApplyAffectors()
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		Rigidbody*& rigidbody = m_rigidbodies[rigidbodyIdx];
		if ( rigidbody->GetSimulationMode() == SIMULATION_MODE_DYNAMIC )
		{
			for ( const auto& affector : m_affectors )
			{
				affector( rigidbody );
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsScene::AddAffector( AffectorFn affectorFunc )
{
	m_affectors.push_back( affectorFunc );
}


//-----------------------------------------------------------------------------------------------
void PhysicsScene::DestroyRigidbody( Rigidbody* rigidbody )
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		Rigidbody*& rigidbodyInList = m_rigidbodies[rigidbodyIdx];
		if ( rigidbody == rigidbodyInList )
		{
			m_garbageRigidbodyIndexes.push_back( rigidbodyIdx );
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsScene::DestroyCollider( Collider* collider )
{
	for ( int colliderIdx = 0; colliderIdx < (int)m_colliders.size(); ++colliderIdx )
	{
		Collider*& colliderInList = m_colliders[colliderIdx];
		if ( collider == colliderInList )
		{
			m_garbageColliderIndexes.push_back( colliderIdx );
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsScene::CleanupDestroyedObjects()
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
void PhysicsScene::DestroyAllRigidbodies()
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		m_garbageRigidbodyIndexes.push_back( rigidbodyIdx );		
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsScene::DestroyAllColliders()
{
	for ( int colliderIdx = 0; colliderIdx < (int)m_colliders.size(); ++colliderIdx )
	{
		m_garbageColliderIndexes.push_back( colliderIdx );
	}
}
