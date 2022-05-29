#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Manifold.hpp"
#include "Engine/Physics/CollisionResolver.hpp"


//-----------------------------------------------------------------------------------------------
class Collider;
class Rigidbody;


//-----------------------------------------------------------------------------------------------
class NullCollision : public CollisionResolver<NullCollision>
{
public:
	static Manifold GetCollisionManifoldForColliders( const Collider* collider, const Collider* otherCollider ) 
	{
		UNUSED( collider ); UNUSED( otherCollider );
		return Manifold();
	}
	static void ApplyCollisionImpulses( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold ) 
	{ 
		UNUSED( rigidbody1 ); UNUSED( rigidbody2 );	UNUSED( collisionManifold ); 
	}
};


//-----------------------------------------------------------------------------------------------
class Simple3DCollision : public CollisionResolver<Simple3DCollision>
{
public:
	static Manifold GetCollisionManifoldForColliders( const Collider* collider, const Collider* otherCollider );
	static void ApplyCollisionImpulses( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold );
};


//-----------------------------------------------------------------------------------------------
class GJK2DCollision : public CollisionResolver<GJK2DCollision>
{
public:
	static Manifold GetCollisionManifoldForColliders( const Collider* collider, const Collider* otherCollider );
	static void ApplyCollisionImpulses( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold );
};
