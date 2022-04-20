#include "Engine/Physics/CollisionResolvers/Simple3DCollisionResolver.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/3D/SphereCollider.hpp"
#include "Engine/Physics/3D/OBB3Collider.hpp"


//-----------------------------------------------------------------------------------------------
typedef bool ( *CollisionCheckCallback )( const Collider*, const Collider* );
typedef Manifold( *CollisionManifoldGenerationCallback )( const Collider*, const Collider* );


//-----------------------------------------------------------------------------------------------
static Manifold SphereVSphereCollisionManifoldGenerator( const Collider* collider1, const Collider* collider2 )
{
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	const SphereCollider* sphereCollider1 = (const SphereCollider*)collider1;
	const SphereCollider* sphereCollider2 = (const SphereCollider*)collider2;
	
	Manifold manifold;
	/*manifold.normal = sphereCollider2->m_worldPosition - sphereCollider1->m_worldPosition;
	manifold.normal.Normalize();

	Vec2 disc1Edge = discCollider1->m_worldPosition + ( manifold.normal * discCollider1->m_radius );
	Vec2 disc2Edge = discCollider2->m_worldPosition + ( -manifold.normal * discCollider2->m_radius );
	manifold.penetrationDepth = GetDistance2D( disc1Edge, disc2Edge );

	manifold.contactPoint1 = disc1Edge - ( manifold.normal * manifold.penetrationDepth * .5f );
	manifold.contactPoint2 = manifold.contactPoint1;*/

	return manifold;
}


//-----------------------------------------------------------------------------------------------
static Manifold SphereVOBB3CollisionManifoldGenerator( const Collider* collider1, const Collider* collider2 )
{
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	const SphereCollider* sphereCollider1 = (const SphereCollider*)collider1;
	const OBB3Collider* obb3Collider1 = (const OBB3Collider*)collider2;

	Manifold manifold;
	

	return manifold;
}


//-----------------------------------------------------------------------------------------------
static Manifold OBB3VOBB3CollisionManifoldGenerator( const Collider* collider1, const Collider* collider2 )
{
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	const OBB3Collider* obb3Collider1 = (const OBB3Collider*)collider1;
	const OBB3Collider* obb3Collider2 = (const OBB3Collider*)collider2;

	Manifold manifold;
	

	return manifold;
}


//-----------------------------------------------------------------------------------------------
// a "matrix" lookup is just a 2D array
static CollisionManifoldGenerationCallback g_ManifoldGenerators[NUM_3D_COLLIDER_TYPES * NUM_3D_COLLIDER_TYPES] = {
	/*            sphere,									obb3, */
	/* sphere */  SphereVSphereCollisionManifoldGenerator,  nullptr,
	/* obb3   */  SphereVOBB3CollisionManifoldGenerator,    OBB3VOBB3CollisionManifoldGenerator
};


//-----------------------------------------------------------------------------------------------
Manifold Simple3DCollisionResolver::GetCollisionManifoldForColliders( const Collider* collider, const Collider* otherCollider )
{
	if ( !collider->IsEnabled()
		 || !otherCollider->IsEnabled() )
		// || !DoAABBsOverlap2D( GetWorldBounds(), other->GetWorldBounds() ) )
	{
		return Manifold();
	}

	eColliderType myType = collider->GetType();
	eColliderType otherType = otherCollider->GetType();

	if ( myType <= otherType )
	{
		int idx = otherType * NUM_3D_COLLIDER_TYPES + myType;
		CollisionManifoldGenerationCallback manifoldGenerator = g_ManifoldGenerators[idx];
		return manifoldGenerator( collider, otherCollider );
	}
	else
	{
		// flip the types when looking into the index.
		int idx = myType * NUM_3D_COLLIDER_TYPES + otherType;
		CollisionManifoldGenerationCallback manifoldGenerator = g_ManifoldGenerators[idx];
		Manifold manifold = manifoldGenerator( otherCollider, collider );
		manifold.normal *= -1.f;
		return manifold;
	}
}


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
