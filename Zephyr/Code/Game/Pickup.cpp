#include "Game/Pickup.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Map.hpp"


//-----------------------------------------------------------------------------------------------
Pickup::Pickup( const EntityDefinition& entityDef, Map* map )
	: Entity( entityDef, map )
{
	//m_rigidbody->SetLayer( eCollisionLayer::PICKUP );

	//m_rigidbody->GetCollider()->m_onTriggerEnterDelegate.SubscribeMethod( this, &Pickup::EnterCollisionEvent );
}


//-----------------------------------------------------------------------------------------------
Pickup::~Pickup()
{
}


//-----------------------------------------------------------------------------------------------
void Pickup::EnterCollisionEvent( Collision collision )
{
	if ( !IsDead() )
	{
		EntityId theirEntityId = collision.theirCollider->GetRigidbody()->m_userProperties.GetValue( "entityId", (EntityId)-1 );

		Entity* theirEntity = g_game->GetEntityById( theirEntityId );
		if ( theirEntity != nullptr )
		{
			m_map->AddItemToTargetInventory( this, theirEntity );
		}
	}
}

