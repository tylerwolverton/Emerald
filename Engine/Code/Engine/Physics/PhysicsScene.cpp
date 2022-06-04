#include "Engine/Physics/PhysicsScene.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Physics/CollisionResolver.hpp"
#include "Engine/Physics/2D/DiscCollider.hpp"
#include "Engine/Physics/2D/PolygonCollider2D.hpp"
#include "Engine/Physics/3D/OBB3Collider.hpp"
#include "Engine/Physics/3D/SphereCollider.hpp"


//-----------------------------------------------------------------------------------------------
void PhysicsScene::DebugRender() const
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)rigidbodies.size(); ++rigidbodyIdx )
	{
		rigidbodies[rigidbodyIdx]->DebugRender( Rgba8::YELLOW, Rgba8::RED );
	}
}


//-----------------------------------------------------------------------------------------------
Rigidbody* PhysicsScene::CreateRigidbody()
{
	Rigidbody* newRigidbody = new Rigidbody();
	rigidbodies.push_back( newRigidbody );

	return newRigidbody;
}


//-----------------------------------------------------------------------------------------------
Collider* PhysicsScene::CreateDiscCollider( float radius, const Vec3& localPosition )
{
	DiscCollider* collider = new DiscCollider( radius, localPosition );
	PhysicsMaterial material;
	material.m_friction = .9f;
	collider->SetPhysicsMaterial( material );
	colliders.push_back( collider );

	return collider;
}


//-----------------------------------------------------------------------------------------------
Collider* PhysicsScene::CreateDiscTrigger( float radius, const Vec3& localPosition /*= Vec3::ZERO */ )
{
	DiscCollider* collider = new DiscCollider( radius, localPosition );
	collider->m_isTrigger = true;
	colliders.push_back( collider );

	return collider;
}


//-----------------------------------------------------------------------------------------------
Collider* PhysicsScene::CreatePolygon2Collider( const Polygon2& polygon, const Vec3& localPosition )
{
	PolygonCollider2D* collider = new PolygonCollider2D( polygon, localPosition );
	PhysicsMaterial material;
	material.m_friction = .9f;
	collider->SetPhysicsMaterial( material );
	colliders.push_back( collider );

	return collider;
}


//-----------------------------------------------------------------------------------------------
Collider* PhysicsScene::CreateCollider( const std::string& type, NamedProperties* params )
{
	Collider* collider = g_colliderFactory->CreateObject( type, params );
	if ( collider == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Can't create collider with unknown type '%s'", type.c_str() ) );
		return nullptr;
	}
	colliders.push_back( collider );
	return collider;
}


//-----------------------------------------------------------------------------------------------
Collider* PhysicsScene::CreateSphereCollider( float radius, const Vec3& localPosition )
{
	SphereCollider* collider = new SphereCollider( radius, localPosition );
	PhysicsMaterial material;
	material.m_friction = .9f;
	collider->SetPhysicsMaterial( material );
	colliders.push_back( collider );
	
	return collider;
}


//-----------------------------------------------------------------------------------------------
Collider* PhysicsScene::CreateOBB3Collider( const OBB3& box, const Vec3& localPosition )
{
	OBB3Collider* collider = new OBB3Collider( box, localPosition );
	PhysicsMaterial material;
	material.m_friction = 1.f;
	collider->SetPhysicsMaterial( material );
	colliders.push_back( collider );

	return collider;
}


//-----------------------------------------------------------------------------------------------
void PhysicsScene::Reset()
{
	DestroyAllColliders();
	DestroyAllRigidbodies();
	CleanupDestroyedObjects();
	affectors.clear();
}


//-----------------------------------------------------------------------------------------------
void PhysicsScene::AddAffector( AffectorFn affectorFunc )
{
	affectors.push_back( affectorFunc );
}


//-----------------------------------------------------------------------------------------------
void PhysicsScene::DestroyRigidbody( Rigidbody* rigidbody )
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)rigidbodies.size(); ++rigidbodyIdx )
	{
		Rigidbody*& rigidbodyInList = rigidbodies[rigidbodyIdx];
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
	for ( int colliderIdx = 0; colliderIdx < (int)colliders.size(); ++colliderIdx )
	{
		Collider*& colliderInList = colliders[colliderIdx];
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
		Rigidbody*& garbageRigidbody = rigidbodies[m_garbageRigidbodyIndexes[rigidbodyIdx]];
		PTR_SAFE_DELETE( garbageRigidbody );
	}

	m_garbageRigidbodyIndexes.clear();

	// Cleanup colliders
	for ( int colliderIdx = 0; colliderIdx < (int)m_garbageColliderIndexes.size(); ++colliderIdx )
	{
		Collider*& garbageCollider = colliders[m_garbageColliderIndexes[colliderIdx]];
		PTR_SAFE_DELETE( garbageCollider );
	}

	m_garbageColliderIndexes.clear();
}


//-----------------------------------------------------------------------------------------------
void PhysicsScene::DestroyAllRigidbodies()
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)rigidbodies.size(); ++rigidbodyIdx )
	{
		m_garbageRigidbodyIndexes.push_back( rigidbodyIdx );		
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsScene::DestroyAllColliders()
{
	for ( int colliderIdx = 0; colliderIdx < (int)colliders.size(); ++colliderIdx )
	{
		m_garbageColliderIndexes.push_back( colliderIdx );
	}
}
