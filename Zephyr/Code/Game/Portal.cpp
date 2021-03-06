#include "Game/Portal.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
Portal::Portal( const EntityDefinition& entityDef, Map* map )
	: Entity( entityDef, map )
{
	m_rigidbody->SetSimulationMode( SIMULATION_MODE_STATIC );
	m_rigidbody->SetLayer( eCollisionLayer::PORTAL );

	m_rigidbody->GetCollider()->m_onTriggerEnterDelegate.SubscribeMethod( this, &Portal::EnterTriggerEvent );
}


//-----------------------------------------------------------------------------------------------
Portal::~Portal()
{
}


////-----------------------------------------------------------------------------------------------
//void Portal::Load()
//{
//	m_rigidbody2D->Enable();
//
//	DiscCollider2D* discCollider = g_physicsSystem2D->CreateDiscTrigger( Vec2::ZERO, GetPhysicsRadius() );
//	m_rigidbody2D->TakeCollider( discCollider );
//
//	m_rigidbody2D->GetCollider()->m_onTriggerEnterDelegate.SubscribeMethod( this, &Portal::EnterTriggerEvent );
//}


//-----------------------------------------------------------------------------------------------
void Portal::EnterTriggerEvent( Collision collision )
{
	if ( !IsDead() )
	{
		EntityId theirEntityId = collision.theirCollider->GetRigidbody()->m_userProperties.GetValue( "entityId", (EntityId)-1 );

		Entity* theirEntity = g_game->GetEntityById( theirEntityId );
		if ( theirEntity != nullptr )
		{
			m_map->WarpEntityInMap( theirEntity, this );
		}
	}
}

