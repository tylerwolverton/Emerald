#pragma once
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/Rigidbody.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct OBB3;
class CollisionResolver;

typedef void ( *AffectorFn )( Rigidbody* rigidbody );


//-----------------------------------------------------------------------------------------------
class PhysicsScene
{
	friend class PhysicsSystem;

public:	
	virtual ~PhysicsScene();
	void DebugRender() const;

	void Reset();

	void ApplyAffectors();
	void AddAffector( AffectorFn affectorFunc );

	void SetCollisionResolver( CollisionResolver* resolver )										{ m_collisionResolver = resolver; }

	// TODO: Make a cylinder def object
	Rigidbody* CreateCylinderRigidbody( const Vec3& worldPosition, float mass, float radius );
	Rigidbody* CreateOBB3Rigidbody( const OBB3& box, float mass );

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

	std::vector<AffectorFn> m_affectors;

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