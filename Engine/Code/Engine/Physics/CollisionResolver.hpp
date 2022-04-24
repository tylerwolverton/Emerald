#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Physics/Manifold.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Collider;
class Collision;
class Rigidbody;


//-----------------------------------------------------------------------------------------------
class Collision
{
public:
	IntVec2 id = IntVec2( -1, -1 );
	uint frameNum = 0;

	Collider* myCollider = nullptr;
	Collider* theirCollider = nullptr;
	Manifold collisionManifold;
};


//-----------------------------------------------------------------------------------------------
enum class eCollisionEventType
{
	ENTER,
	STAY,
	LEAVE
};


//-----------------------------------------------------------------------------------------------
class CollisionResolver
{
public:
	virtual ~CollisionResolver() {};

	virtual void ResolveCollisions( std::vector<Collider*>& colliders, uint frameNum );

protected:
	void DetectCollisions( const std::vector<Collider*>& colliders, uint frameNum );
	void ClearOldCollisions( uint frameNum );
	void ResolveCollisions();
	void ResolveCollision( const Collision& collision );
	virtual void CorrectCollidingRigidbodies( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold );

	virtual Manifold GetCollisionManifoldForColliders( const Collider* collider, const Collider* otherCollider ) = 0;
	virtual void ApplyCollisionImpulses( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold ) = 0;

	void InvokeCollisionEvents( const Collision& collision, eCollisionEventType collisionType ) const;
	void AddOrUpdateCollision( const Collision& collision );

protected:
	std::vector<Collision> collisions;
};
