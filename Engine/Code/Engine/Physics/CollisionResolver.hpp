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
	virtual ~CollisionResolver() {};

	virtual void ResolveCollisions( std::vector<Collider*>& colliders, uint frameNum );

protected:
	void DetectCollisions( const std::vector<Collider*>& colliders, uint frameNum );
	void ClearOldCollisions( uint frameNum );
	void ResolveCollisions();
	void ResolveCollision( const Collision& collision );
	virtual void CorrectCollidingRigidbodies( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold );

	void InvokeCollisionEvents( const Collision& collision, eCollisionEventType collisionType ) const;
	void AddOrUpdateCollision( const Collision& collision );

protected:
	std::vector<Collision> collisions;
};

#include "Engine/Physics/CollisionResolver.inl"
