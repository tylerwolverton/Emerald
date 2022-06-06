#pragma once
#include "Engine/Core/ObjectFactory.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Physics/Rigidbody.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct PhysicsScene 
{
public:
	AffectorMap affectors;

	RigidbodyVector rigidbodies;
	ColliderVector colliders;
	CollisionVector collisions;

private:
	std::vector<int> m_garbageRigidbodyIndexes;
	std::vector<int> m_garbageColliderIndexes;

public:
	void DebugRender() const;

	void Reset();

	void AddAffector( const std::string& name, AffectorFn affectorFunc );
	void RemoveAffector( const std::string& name );

	Rigidbody* CreateRigidbody();
	Collider* CreateCollider( const ColliderId& type, ColliderParams* params );
	Collider* CreateTrigger( const ColliderId& type, ColliderParams* params );

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