#pragma once
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Physics/CollisionResolver.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct OBB3;

typedef void ( *AffectorFn )( Rigidbody* rigidbody );
typedef std::vector<Rigidbody*> RigidbodyVector;
typedef std::vector<Collider*> ColliderVector;
typedef std::vector<AffectorFn> AffectorVector;


//-----------------------------------------------------------------------------------------------
class PhysicsSceneBase
{
	friend class PhysicsSystem;

public:
	virtual ~PhysicsSceneBase();
	void DebugRender() const;

	void Reset();

	void AddAffector( AffectorFn affectorFunc );

	virtual void ResolveCollisions( int frameNum ) = 0;
	//void SetCollisionResolver( CollisionResolver* resolver )										{ m_collisionResolver = resolver; }

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

protected:
	// TODO: Change these from pointers to flat data
	RigidbodyVector m_rigidbodies;
	std::vector<Collider*> m_colliders;

	std::vector<int> m_garbageRigidbodyIndexes;
	std::vector<int> m_garbageColliderIndexes;

	AffectorVector m_affectors;
	//template <class CollisionPolicy>
};


//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
class PhysicsScene : public PhysicsSceneBase
{
public:
	virtual void ResolveCollisions( int frameNum ) override;

private:
	template <class CollisionPolicy>
	CollisionResolver<CollisionPolicy> m_collisionResolver;// = nullptr;
};


template <class CollisionPolicy>
void PhysicsScene<CollisionPolicy>::ResolveCollisions( int frameNum )
{
	m_collisionResolver.ResolveCollisions( m_colliders, frameNum );
}

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