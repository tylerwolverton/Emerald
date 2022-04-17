#pragma once
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/Rigidbody.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class CollisionResolver;


//-----------------------------------------------------------------------------------------------
class PhysicsScene
{
	friend class PhysicsSystem;

public:	
	~PhysicsScene();

	// TODO: Make a cylinder def object
	Rigidbody* CreateCylinderRigidbody( const Vec3& worldPosition, float mass, float radius );

	void Reset();

	void DestroyRigidbody( Rigidbody* rigidbody );
	void DestroyCollider( Collider* collider );

	void CleanupDestroyedObjects();

private:
	void DestroyAllRigidbodies();
	void DestroyAllColliders();

private:
	// TODO: Change these from pointers to flat data
	std::vector<Rigidbody*> m_rigidbodies;
	std::vector<Collider*> m_colliders;
	
	std::vector<int> m_garbageRigidbodyIndexes;
	std::vector<int> m_garbageColliderIndexes;

	CollisionResolver* m_collisionResolver = nullptr;
};

// ApplyAffectors
//  Data: Rigidbodies, Affectors
//  Customize: Yes
//  Nullable: Yes
//
// MoveRigidbodies
//  Data: Rigidbodies
//  Customize: No
//  Nullable: No
//
// ResolveCollisions
//  CorrectPenetration
//  AddImpulses
//  FireCollisionEvents
// Data: Colliders, Collisions
//  Customize: Yes
//  Nullable: Yes
//
// UpdateRigidbodies -> new position after colliders moved
// Data: Colliders, Rigidbodies
//  Customize: No
//  Nullable: No
//
// CleanupGarbageObjects
//  Data: Rigidbodies, Colliders, (Collisions?)
//  Customize: No
//  Nullable: No
//
//