#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Physics/Manifold.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Collider;
class Collision;
class Rigidbody;


//-----------------------------------------------------------------------------------------------
enum class eCollisionEventType
{
	ENTER,
	STAY,
	LEAVE
};


//-----------------------------------------------------------------------------------------------
template <class CollisionPolicy>
class CollisionResolver
{
public:
	static void ResolveCollisions( std::vector<Collider*>& colliders, CollisionVector& collisions, uint frameNum );

protected:
	static void DetectCollisions( const std::vector<Collider*>& colliders, CollisionVector& collisions, uint frameNum );
	static void ClearOldCollisions( CollisionVector& collisions, uint frameNum );
	static void ResolveCollisions( CollisionVector& collisions );
	static void ResolveCollision( const Collision& collision );
	static void CorrectCollidingRigidbodies( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold );
	 
	static void InvokeCollisionEvents( const Collision& collision, eCollisionEventType collisionType );
	static void AddOrUpdateCollision( CollisionVector& collisions, const Collision& collision );

	static bool DoesCollisionInvolveATrigger( const Collision& collision );
};

#include "Engine/Physics/CollisionResolver.inl"
