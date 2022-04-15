#include "Engine/Physics/CollisionResolver.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/Collision.hpp"


//-----------------------------------------------------------------------------------------------
void CollisionResolver::ResolveCollisions( std::vector<Collider*>& colliders )
{
	DetectCollisions( colliders );					// determine all pairs of intersecting colliders
	ClearOldCollisions();

	//ResolveCollision( const Collision & collision );
}


//-----------------------------------------------------------------------------------------------
bool DoesCollisionInvolveATrigger( const Collision& collision )
{
	return collision.myCollider->m_isTrigger
		|| collision.theirCollider->m_isTrigger;
}


//-----------------------------------------------------------------------------------------------
void CollisionResolver::DetectCollisions( const std::vector<Collider*>& colliders )
{
	for ( int colliderIdx = 0; colliderIdx < (int)colliders.size(); ++colliderIdx )
	{
		Collider* collider = colliders[colliderIdx];
		if ( collider == nullptr
			 || !collider->IsEnabled() )
		{
			continue;
		}

		// Check intersection with other game objects
		for ( int otherColliderIdx = colliderIdx + 1; otherColliderIdx < (int)colliders.size(); ++otherColliderIdx )
		{
			Collider* otherCollider = colliders[otherColliderIdx];
			if ( otherCollider == nullptr
				 || !otherCollider->IsEnabled() )
			{
				continue;
			}

			// Skip if colliders have same parent rigidbody
			if ( collider->GetRigidbody() == otherCollider->GetRigidbody() )
			{
				continue;
			}

			// Skip if colliders on non-interacting layers
			if ( !DoLayersInteract( collider->GetRigidbody()->GetLayer(),
									otherCollider->GetRigidbody()->GetLayer() ) )
			{
				continue;
			}

			// TODO: Remove Intersects check
			if ( collider->Intersects( otherCollider ) )
			{
				Collision collision;
				collision.id = IntVec2( Min( collider->GetId(), otherCollider->GetId() ), Max( collider->GetId(), otherCollider->GetId() ) );
				collision.frameNum = m_frameNum;
				collision.myCollider = collider;
				collision.theirCollider = otherCollider;
				// Only calculate manifold if not triggers
				if ( !DoesCollisionInvolveATrigger( collision ) )
				{
					collision.collisionManifold = collider->GetCollisionManifold( otherCollider );
				}

				AddOrUpdateCollision( collision );
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void CollisionResolver::ClearOldCollisions()
{

}


//-----------------------------------------------------------------------------------------------
void CollisionResolver::ResolveCollisions()
{

}


//-----------------------------------------------------------------------------------------------
void CollisionResolver::ResolveCollision( const Collision& collision )
{

}



