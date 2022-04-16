#include "Engine/Physics/CollisionResolver.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/Rigidbody.hpp"


//-----------------------------------------------------------------------------------------------
void CollisionResolver::ResolveCollisions( std::vector<Collider*>& colliders, uint frameNum )
{
	DetectCollisions( colliders, frameNum );					// determine all pairs of intersecting colliders
	ClearOldCollisions( frameNum );

	ResolveCollisions( colliders, frameNum );
}


//-----------------------------------------------------------------------------------------------
bool DoesCollisionInvolveATrigger( const Collision& collision )
{
	return collision.myCollider->IsTrigger()
		|| collision.theirCollider->IsTrigger();
}


//-----------------------------------------------------------------------------------------------
void CollisionResolver::DetectCollisions( const std::vector<Collider*>& colliders, uint frameNum )
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
			if ( !DoPhysicsLayersInteract( collider->GetRigidbody()->GetLayer(),
									otherCollider->GetRigidbody()->GetLayer() ) )
			{
				continue;
			}

			// TODO: Remove Intersects check
			if ( collider->Intersects( otherCollider ) )
			{
				Collision collision;
				collision.id = IntVec2( Min( collider->GetId(), otherCollider->GetId() ), Max( collider->GetId(), otherCollider->GetId() ) );
				collision.frameNum = frameNum;
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
void CollisionResolver::InvokeCollisionEvents( const Collision& collision, eCollisionEventType collisionType ) const
{
	// Inverse collision from perspective of their collider
	Collision theirCollision = collision;
	theirCollision.myCollider = collision.theirCollider;
	theirCollision.theirCollider = collision.myCollider;

	// Fire correct enter events
	if ( !DoesCollisionInvolveATrigger( collision ) )
	{
		switch ( collisionType )
		{
			case eCollisionEventType::ENTER:
			{
				collision.myCollider->m_onOverlapEnterDelegate.Invoke( collision );
				collision.theirCollider->m_onOverlapEnterDelegate.Invoke( theirCollision );
			}
			break;

			case eCollisionEventType::STAY:
			{
				collision.myCollider->m_onOverlapStayDelegate.Invoke( collision );
				collision.theirCollider->m_onOverlapStayDelegate.Invoke( theirCollision );
			}
			break;

			case eCollisionEventType::LEAVE:
			{
				if ( collision.myCollider != nullptr )
				{
					collision.myCollider->m_onOverlapLeaveDelegate.Invoke( collision );
				}
				if ( collision.theirCollider != nullptr )
				{
					collision.theirCollider->m_onOverlapLeaveDelegate.Invoke( theirCollision );
				}
			}
			break;
		}
	}
	else
	{
		if ( collision.myCollider->IsTrigger() )
		{
			switch ( collisionType )
			{
				case eCollisionEventType::ENTER: collision.myCollider->m_onTriggerEnterDelegate.Invoke( collision ); break;
				case eCollisionEventType::STAY:	 collision.myCollider->m_onTriggerStayDelegate.Invoke( collision ); break;
				case eCollisionEventType::LEAVE: collision.myCollider->m_onTriggerLeaveDelegate.Invoke( collision ); break;
			}
		}

		if ( collision.theirCollider->IsTrigger() )
		{
			switch ( collisionType )
			{
				case eCollisionEventType::ENTER: collision.theirCollider->m_onTriggerEnterDelegate.Invoke( theirCollision ); break;
				case eCollisionEventType::STAY:  collision.theirCollider->m_onTriggerStayDelegate.Invoke( theirCollision ); break;
				case eCollisionEventType::LEAVE: collision.theirCollider->m_onTriggerLeaveDelegate.Invoke( theirCollision ); break;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void CollisionResolver::AddOrUpdateCollision( const Collision& collision )
{
	for ( int colIdx = 0; colIdx < (int)collisions.size(); ++colIdx )
	{
		// Check if collision is already in progress
		if ( collisions[colIdx].id == collision.id )
		{
			InvokeCollisionEvents( collision, eCollisionEventType::STAY );
			collisions[colIdx] = collision;
			return;
		}
	}

	InvokeCollisionEvents( collision, eCollisionEventType::ENTER );
	collisions.push_back( collision );
}


//-----------------------------------------------------------------------------------------------
void CollisionResolver::ClearOldCollisions( uint frameNum )
{
	std::vector<int> oldCollisionIds;

	for ( int colIdx = 0; colIdx < (int)collisions.size(); ++colIdx )
	{
		Collision& collision = collisions[colIdx];
		// Check if collision is old
		if ( collision.frameNum != frameNum )
		{
			InvokeCollisionEvents( collision, eCollisionEventType::LEAVE );
			//collision.myCollider->m_rigidbody->m_collider = nullptr;
			//collision.theirCollider->m_rigidbody->m_collider = nullptr;
			collision.myCollider = nullptr;
			collision.theirCollider = nullptr;
			oldCollisionIds.push_back( colIdx );
		}
	}

	for ( int oldColIdx = (int)oldCollisionIds.size() - 1; oldColIdx >= 0; --oldColIdx )
	{
		int idxToRemove = oldCollisionIds[oldColIdx];
		collisions.erase( collisions.begin() + idxToRemove );
	}
}


//-----------------------------------------------------------------------------------------------
void CollisionResolver::ResolveCollisions()
{
	for ( int collisionIdx = 0; collisionIdx < (int)collisions.size(); ++collisionIdx )
	{
		Collision& collision = collisions[collisionIdx];
		if ( !DoesCollisionInvolveATrigger( collision ) )
		{
			ResolveCollision( collision );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void CollisionResolver::ResolveCollision( const Collision& collision )
{
	Rigidbody* myRigidbody = collision.myCollider->GetRigidbody();
	Rigidbody* theirRigidbody = collision.theirCollider->GetRigidbody();

	GUARANTEE_OR_DIE( myRigidbody != nullptr, "My Collider doesn't have a rigidbody" );
	GUARANTEE_OR_DIE( theirRigidbody != nullptr, "Their Collider doesn't have a rigidbody" );

	// Do nothing when both are static
	if ( myRigidbody->GetSimulationMode() == SIMULATION_MODE_STATIC
		 && theirRigidbody->GetSimulationMode() == SIMULATION_MODE_STATIC )
	{
		return;
	}

	CorrectCollidingRigidbodies( myRigidbody, theirRigidbody, collision.collisionManifold );
	ApplyCollisionImpulses( myRigidbody, theirRigidbody, collision.collisionManifold );
}




