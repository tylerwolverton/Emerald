#include "Engine/Physics/PhysicsScene.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Physics/CollisionResolver.hpp"
#include "Engine/Physics/2D/DiscCollider.hpp"
#include "Engine/Physics/2D/PolygonCollider2D.hpp"
#include "Engine/Physics/3D/OBB3Collider.hpp"
#include "Engine/Physics/3D/SphereCollider.hpp"


//-----------------------------------------------------------------------------------------------
PhysicsSceneBase::~PhysicsSceneBase()
{
	Reset();
}


//-----------------------------------------------------------------------------------------------
void PhysicsSceneBase::DebugRender() const
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		m_rigidbodies[rigidbodyIdx]->DebugRender( Rgba8::YELLOW, Rgba8::RED );
	}
}


//-----------------------------------------------------------------------------------------------
Rigidbody* PhysicsSceneBase::CreateRigidbody()
{
	Rigidbody* newRigidbody = new Rigidbody();
	m_rigidbodies.push_back( newRigidbody );

	return newRigidbody;
}


//-----------------------------------------------------------------------------------------------
Collider* PhysicsSceneBase::CreateDiscCollider( float radius, const Vec3& localPosition )
{
	DiscCollider* collider = new DiscCollider( radius, localPosition );
	PhysicsMaterial material;
	material.m_friction = .9f;
	collider->SetPhysicsMaterial( material );
	m_colliders.push_back( collider );

	return collider;
}


//-----------------------------------------------------------------------------------------------
Collider* PhysicsSceneBase::CreateDiscTrigger( float radius, const Vec3& localPosition /*= Vec3::ZERO */ )
{
	DiscCollider* collider = new DiscCollider( radius, localPosition );
	collider->m_isTrigger = true;
	m_colliders.push_back( collider );

	return collider;
}


//-----------------------------------------------------------------------------------------------
Collider* PhysicsSceneBase::CreatePolygon2Collider( const Polygon2& polygon, const Vec3& localPosition )
{
	PolygonCollider2D* collider = new PolygonCollider2D( polygon, localPosition );
	PhysicsMaterial material;
	material.m_friction = .9f;
	collider->SetPhysicsMaterial( material );
	m_colliders.push_back( collider );

	return collider;
}


//-----------------------------------------------------------------------------------------------
Collider* PhysicsSceneBase::CreateSphereCollider( float radius, const Vec3& localPosition )
{
	SphereCollider* collider = new SphereCollider( radius, localPosition );
	PhysicsMaterial material;
	material.m_friction = .9f;
	collider->SetPhysicsMaterial( material );
	m_colliders.push_back( collider );
	
	return collider;
}


//-----------------------------------------------------------------------------------------------
Collider* PhysicsSceneBase::CreateOBB3Collider( const OBB3& box, const Vec3& localPosition )
{
	OBB3Collider* collider = new OBB3Collider( box, localPosition );
	PhysicsMaterial material;
	material.m_friction = 1.f;
	collider->SetPhysicsMaterial( material );
	m_colliders.push_back( collider );

	return collider;
}


//-----------------------------------------------------------------------------------------------
void PhysicsSceneBase::Reset()
{
	DestroyAllColliders();
	DestroyAllRigidbodies();
	CleanupDestroyedObjects();
	m_affectors.clear();
}


//-----------------------------------------------------------------------------------------------
void PhysicsSceneBase::ApplyAffectors()
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
void PhysicsSceneBase::AddAffector( AffectorFn affectorFunc )
{
	m_affectors.push_back( affectorFunc );
}


//-----------------------------------------------------------------------------------------------
void PhysicsSceneBase::DestroyRigidbody( Rigidbody* rigidbody )
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
void PhysicsSceneBase::DestroyCollider( Collider* collider )
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
void PhysicsSceneBase::CleanupDestroyedObjects()
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
void PhysicsSceneBase::DestroyAllRigidbodies()
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		m_garbageRigidbodyIndexes.push_back( rigidbodyIdx );		
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSceneBase::DestroyAllColliders()
{
	for ( int colliderIdx = 0; colliderIdx < (int)m_colliders.size(); ++colliderIdx )
	{
		m_garbageColliderIndexes.push_back( colliderIdx );
	}
}
