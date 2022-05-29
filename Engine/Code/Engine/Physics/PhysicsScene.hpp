#pragma once
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Physics/Rigidbody.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct OBB3;


//-----------------------------------------------------------------------------------------------
struct PhysicsScene 
{
public:
	AffectorVector affectors;

	RigidbodyVector rigidbodies;
	ColliderVector colliders;
	CollisionVector collisions;

protected:
	std::vector<int> m_garbageRigidbodyIndexes;
	std::vector<int> m_garbageColliderIndexes;

public:
	void DebugRender() const;

	void Reset();

	void AddAffector( AffectorFn affectorFunc );

	// TODO: Move to factory policy
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