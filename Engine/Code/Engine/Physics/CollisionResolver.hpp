#pragma once
#include <vector>

struct Manifold;
class Collider;
class Collision;
class Rigidbody;

class CollisionResolver
{
public:
	virtual void ResolveCollisions( std::vector<Collider*>& colliders );

protected:
	void DetectCollisions( const std::vector<Collider*>& colliders );
	void ClearOldCollisions();
	void ResolveCollisions();
	void ResolveCollision( const Collision& collision );
	virtual void CorrectCollidingRigidbodies( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold ) = 0;
	virtual void ApplyCollisionImpulses( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold ) = 0;

protected:
	std::vector<Collision> collisions;
};
