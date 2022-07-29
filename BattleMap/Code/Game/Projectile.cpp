#include "Game/Projectile.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/ZephyrCore/ZephyrScript.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"


//-----------------------------------------------------------------------------------------------
Projectile::Projectile( const EntityDefinition& entityDef, Map* map )
	: Entity( entityDef, map )
{
	m_damage = entityDef.GetDamageRange().GetRandomInRange( g_game->m_rng );

	m_rigidbody->SetDrag( 0.f );
}


//-----------------------------------------------------------------------------------------------
Projectile::~Projectile()
{
}


//-----------------------------------------------------------------------------------------------
void Projectile::Update( float deltaSeconds )
{
	m_rigidbody->SetVelocity( Vec3( GetForwardVector(), 0.f ) * m_entityDef.GetSpeed() );

	Entity::Update( deltaSeconds );
}

