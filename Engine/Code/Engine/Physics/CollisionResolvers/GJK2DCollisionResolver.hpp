#pragma once
#include "Engine/Physics/CollisionResolver.hpp"


//-----------------------------------------------------------------------------------------------
class GJK2DCollisionResolver : public CollisionResolver
{
public:
	virtual ~GJK2DCollisionResolver() {};

protected:
	virtual Manifold GetCollisionManifoldForColliders( const Collider* collider, const Collider* otherCollider ) override;
	virtual void ApplyCollisionImpulses( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold ) override;
};
