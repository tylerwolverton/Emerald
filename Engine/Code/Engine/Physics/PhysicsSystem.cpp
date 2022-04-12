#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/Collision.hpp"
#include "Engine/Physics/Manifold.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Timer.hpp"


//-----------------------------------------------------------------------------------------------
static float s_fixedDeltaSeconds = 1.0f / 120.0f;


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::Startup( Clock* gameClock )
{
	m_gameClock = gameClock;
	if ( m_gameClock == nullptr )
	{
		m_gameClock = Clock::GetMaster();
	}

	m_physicsClock = new Clock( m_gameClock );
	m_stepTimer = new Timer( m_physicsClock );

	m_stepTimer->SetSeconds( s_fixedDeltaSeconds );

	g_eventSystem->RegisterEvent( "set_physics_update", "Usage: set_physics_update hz=NUMBER .Set rate of physics update in hz.", eUsageLocation::DEV_CONSOLE, SetPhysicsUpdateRate );

	// Initialize layers
	for ( int layerIdx = 0; layerIdx < 32; ++layerIdx )
	{
		m_layerInteractions[layerIdx] = ~0U;
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::Update()
{
	while ( m_stepTimer->CheckAndDecrement() )
	{
		AdvanceSimulation( s_fixedDeltaSeconds );
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::AdvanceSimulation( float deltaSeconds )
{
	ApplyEffectors(); 					// apply gravity (or other scene wide effects) to all dynamic objects
	MoveRigidbodies( deltaSeconds ); 	// apply an euler step to all rigidbodies, and reset per-frame data
	DetectCollisions();					// determine all pairs of intersecting colliders
	ClearOldCollisions();
	ResolveCollisions(); 				// resolve all collisions, firing appropraite events
	CleanupDestroyedObjects();  		// destroy objects 

	++m_frameNum;
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::MoveRigidbodies( float deltaSeconds )
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		Rigidbody*& rigidbody = m_rigidbodies[rigidbodyIdx];
		if ( rigidbody != nullptr )
		{
			switch ( rigidbody->GetSimulationMode() )
			{
				case SIMULATION_MODE_DYNAMIC:
				case SIMULATION_MODE_KINEMATIC:
				{
					rigidbody->Update( deltaSeconds );
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
bool PhysicsSystem::DoesCollisionInvolveATrigger( const Collision& collision ) const
{
	return collision.myCollider->m_isTrigger
		|| collision.theirCollider->m_isTrigger;
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::InvokeCollisionEvents( const Collision& collision, eCollisionEventType collisionType ) const
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
		if ( collision.myCollider->m_isTrigger )
		{
			switch ( collisionType )
			{
				case eCollisionEventType::ENTER: collision.myCollider->m_onTriggerEnterDelegate.Invoke( collision ); break;
				case eCollisionEventType::STAY:	 collision.myCollider->m_onTriggerStayDelegate.Invoke( collision ); break;
				case eCollisionEventType::LEAVE: collision.myCollider->m_onTriggerLeaveDelegate.Invoke( collision ); break;
			}
		}

		if ( collision.theirCollider->m_isTrigger )
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
void PhysicsSystem::DetectCollisions()
{
	for ( int colliderIdx = 0; colliderIdx < (int)m_colliders.size(); ++colliderIdx )
	{
		Collider* collider = m_colliders[colliderIdx];
		if ( collider == nullptr 
			 || !collider->IsEnabled() )
		{
			continue;
		}
		
		// Check intersection with other game objects
		for ( int otherColliderIdx = colliderIdx + 1; otherColliderIdx < (int)m_colliders.size(); ++otherColliderIdx )
		{
			Collider* otherCollider = m_colliders[otherColliderIdx];
			if ( otherCollider == nullptr
				 || !otherCollider->IsEnabled() )
			{
				continue;
			}

			// Skip if colliders on non-interacting layers
			if ( !DoLayersInteract( collider->m_rigidbody->GetLayer(), 
									otherCollider->m_rigidbody->GetLayer() ) )
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
void PhysicsSystem::ClearOldCollisions()
{
	std::vector<int> oldCollisionIds;

	for ( int colIdx = 0; colIdx < (int)m_collisions.size(); ++colIdx )
	{
		Collision& collision = m_collisions[colIdx];
		// Check if collision is old
		if ( collision.frameNum != m_frameNum )
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
		m_collisions.erase( m_collisions.begin() + idxToRemove );
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::AddOrUpdateCollision( const Collision& collision )
{
	for ( int colIdx = 0; colIdx < (int)m_collisions.size(); ++colIdx )
	{
		// Check if collision is already in progress
		if ( m_collisions[colIdx].id == collision.id )
		{
			InvokeCollisionEvents( collision, eCollisionEventType::STAY );
			m_collisions[colIdx] = collision;
			return;
		}
	}

	InvokeCollisionEvents( collision, eCollisionEventType::ENTER );
	m_collisions.push_back( collision );
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::DestroyAllRigidbodies()
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_rigidbodies.size(); ++rigidbodyIdx )
	{
		m_garbageRigidbodyIndexes.push_back( rigidbodyIdx );		
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::DestroyAllColliders()
{
	for ( int colliderIdx = 0; colliderIdx < (int)m_colliders.size(); ++colliderIdx )
	{
		m_garbageColliderIndexes.push_back( colliderIdx );
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::ResolveCollisions()
{
	for ( int collisionIdx = 0; collisionIdx < (int)m_collisions.size(); ++collisionIdx )
	{
		Collision2D& collision = m_collisions[collisionIdx];
		if ( !DoesCollisionInvolveATrigger( collision ) )
		{
			ResolveCollision( collision );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::ResolveCollision( const Collision& collision )
{
	Rigidbody* myRigidbody = collision.myCollider->m_rigidbody;
	Rigidbody* theirRigidbody = collision.theirCollider->m_rigidbody;

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


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::CorrectCollidingRigidbodies( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold )
{
	if ( rigidbody1->GetSimulationMode() == SIMULATION_MODE_STATIC
		 || ( rigidbody1->GetSimulationMode() == SIMULATION_MODE_KINEMATIC && rigidbody2->GetSimulationMode() == SIMULATION_MODE_DYNAMIC ) )
	{
		rigidbody2->Translate( collisionManifold.penetrationDepth * collisionManifold.normal );
		return;
	}
	else if ( rigidbody2->GetSimulationMode() == SIMULATION_MODE_STATIC
			  || ( rigidbody2->GetSimulationMode() == SIMULATION_MODE_KINEMATIC && rigidbody1->GetSimulationMode() == SIMULATION_MODE_DYNAMIC ) )
	{
		rigidbody1->Translate( collisionManifold.penetrationDepth * -collisionManifold.normal );
		return;
	}

	float sumOfMasses = rigidbody1->GetMass() + rigidbody2->GetMass();
	float rigidbody1CorrectionDist = ( rigidbody2->GetMass() / sumOfMasses ) * collisionManifold.penetrationDepth;
	float rigidbody2CorrectionDist = ( rigidbody1->GetMass() / sumOfMasses ) * collisionManifold.penetrationDepth;

	rigidbody1->Translate( rigidbody1CorrectionDist * -collisionManifold.normal );
	rigidbody2->Translate( rigidbody2CorrectionDist * collisionManifold.normal );
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::CleanupDestroyedObjects()
{
	// Cleanup rigidbodies
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_garbageRigidbodyIndexes.size(); ++rigidbodyIdx )
	{
		Rigidbody*& garbageRigidbody = m_rigidbodies[m_garbageRigidbodyIndexes[rigidbodyIdx]];
		PTR_SAFE_DELETE( garbageRigidbody );
	}

	m_garbageRigidbodyIndexes.clear();

	// Cleanup colliders
	for ( int colliderIdx = 0; colliderIdx < (int)m_garbageColliderIndexes.size(); ++colliderIdx )
	{
		Collider*& garbageCollider = m_colliders[m_garbageColliderIndexes[colliderIdx]];
		PTR_SAFE_DELETE( garbageCollider );
	}

	m_garbageColliderIndexes.clear();
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::Shutdown()
{
	PTR_SAFE_DELETE( m_stepTimer );
	PTR_SAFE_DELETE( m_physicsClock );
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::Reset()
{
	ClearOldCollisions();

	DestroyAllColliders();
	DestroyAllRigidbodies();
	CleanupDestroyedObjects();

	m_colliders.clear();
	m_rigidbodies.clear();
}


//-----------------------------------------------------------------------------------------------
bool PhysicsSystem::DoLayersInteract( uint layer0, uint layer1 ) const
{
	return ( m_layerInteractions[layer0] & ( 1 << layer1 ) ) != 0 ;
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::EnableLayerInteraction( uint layer0, uint layer1 )
{
	m_layerInteractions[layer0] |= ( 1 << layer1 );
	m_layerInteractions[layer1] |= ( 1 << layer0 );
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::DisableLayerInteraction( uint layer0, uint layer1 )
{
	m_layerInteractions[layer0] &= ~( 1 << layer1 );
	m_layerInteractions[layer1] &= ~( 1 << layer0 );
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::DisableAllLayerInteraction( uint layer )
{
	m_layerInteractions[layer] = 0U;

	for ( int layerIdx = 0; layerIdx < 32; ++layerIdx )
	{
		m_layerInteractions[layerIdx] &= ~( 1 << layer );
	}
}


//-----------------------------------------------------------------------------------------------
float PhysicsSystem::GetFixedDeltaSeconds() const
{
	return s_fixedDeltaSeconds;
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::SetFixedDeltaSeconds( float newDeltaSeconds )
{
	s_fixedDeltaSeconds = newDeltaSeconds;
}


//-----------------------------------------------------------------------------------------------
void PhysicsSystem::ResetFixedDeltaSecondsToDefault()
{
	SetFixedDeltaSeconds( 1.f / 120.f );
}


//-----------------------------------------------------------------------------------------------
bool PhysicsSystem::SetPhysicsUpdateRate( EventArgs* args )
{
	float hz = args->GetValue( "hz", 120.f );

	if ( hz == 0.f )
	{
		s_fixedDeltaSeconds = 0.f;
	}
	else
	{
		s_fixedDeltaSeconds = 1.f / hz;
	}

	return false;
}
