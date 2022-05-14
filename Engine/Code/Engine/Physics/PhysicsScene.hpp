#pragma once
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/Rigidbody.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct OBB3;
template <class CollisionPolicy>
class CollisionResolver;

typedef void ( *AffectorFn )( Rigidbody* rigidbody );


//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
class PhysicsScene
{
	friend class PhysicsSystem;

public:	
	virtual ~PhysicsScene();
	void DebugRender() const;

	void Reset();

	void ApplyAffectors();
	void AddAffector( AffectorFn affectorFunc );

	void ResolveCollisions( int frameNum );
	//void SetCollisionResolver( CollisionResolver* resolver )										{ m_collisionResolver = resolver; }

	// TODO: Make a cylinder def object
	Rigidbody* CreateRigidbody();

	// 2D
	Collider* CreateDiscCollider( float radius, const Vec3& localPosition = Vec3::ZERO );
	Collider* CreateDiscTrigger( float radius, const Vec3& localPosition = Vec3::ZERO );
	Collider* CreatePolygon2Collider( const Polygon2& polygon, const Vec3& localPosition = Vec3::ZERO );

	// 3D
	Collider* CreateSphereCollider( float radius, const Vec3& localPosition = Vec3::ZERO );
	Collider* CreateOBB3Collider( const OBB3& box, const Vec3& localPosition = Vec3::ZERO );

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
	//template <class CollisionPolicy>
	CollisionResolver<CollisionPolicy> m_collisionResolver;// = nullptr;
};

#include "Engine/Physics/PhysicsScene.inl"

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