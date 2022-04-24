#pragma once
#include "Engine/Physics/CollisionResolver.hpp"


class Simple3DCollisionResolver : public CollisionResolver
{
public:
	virtual ~Simple3DCollisionResolver() {};

protected:
	virtual Manifold GetCollisionManifoldForColliders( const Collider* collider, const Collider* otherCollider ) override;
	virtual void ApplyCollisionImpulses( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold ) override;
};
