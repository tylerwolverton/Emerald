#include "Engine/Physics/CollisionResolvers/Simple3DCollisionResolver.hpp"


//-----------------------------------------------------------------------------------------------
void Simple3DCollisionResolver::CorrectCollidingRigidbodies( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold )
{

}


//-----------------------------------------------------------------------------------------------
void Simple3DCollisionResolver::ApplyCollisionImpulses( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold )
{
	UNUSED( rigidbody1 );
	UNUSED( rigidbody2 );
	UNUSED( collisionManifold );
}
