#pragma once
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/Rigidbody.hpp"

#include <vector>

class CollisionResolver;

struct PhysicsScene
{
public:
	std::vector<Rigidbody> rigidbodies;
	//std::vector<int> m_garbageRigidbodyIndexes;
	std::vector<Collider*> colliders;
	//std::vector<int> m_garbageColliderIndexes;

	CollisionResolver* collisionResolver = nullptr;

public:	
	void Reset();

private:

private:
	void DestroyAllRigidbodies();
	void DestroyAllColliders();
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