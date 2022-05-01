#include "Game/Map.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Physics/PhysicsScene.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Physics/CollisionResolvers/Simple3DCollisionResolver.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/Entity.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/MapData.hpp"
#include "Game/World.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( const MapData& mapData, World* world )
	: m_name( mapData.mapName )
	, m_playerStartPos( mapData.playerStartPos )
	, m_playerStartYaw( mapData.playerStartYaw )
	, m_world( world )
{
	m_physicsScene = new PhysicsScene();
	m_curCollisionResolver = new Simple3DCollisionResolver();
	m_physicsScene->SetCollisionResolver( m_curCollisionResolver );

	LoadEntities( mapData.mapEntityDefs );
}


//-----------------------------------------------------------------------------------------------
Map::~Map()
{
	PTR_SAFE_DELETE( m_physicsScene );
	PTR_SAFE_DELETE( m_curCollisionResolver );
	PTR_VECTOR_SAFE_DELETE( m_entities );
}


//-----------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->Update( deltaSeconds );
	}

	g_game->GetCurrentPhysicsSystem()->Update( *m_physicsScene );
	UpdateMeshes();
}


//-----------------------------------------------------------------------------------------------
void Map::Render() const
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->Render();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::DebugRender() const
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->DebugRender();
		/*DebugAddWorldWireCylinder( Vec3( entity->GetPosition(), 0.f ), Vec3( entity->GetPosition(), entity->GetHeight() ), 
								   entity->GetPhysicsRadius(), Rgba8::CYAN );*/
	}

	m_physicsScene->DebugRender();
}


//-----------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntityOfType( const std::string& entityDefName )
{
	EntityDefinition* entityDef = EntityDefinition::GetEntityDefinition( entityDefName );
	if ( entityDef == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Tried to spawn unrecognized entity '%s'", entityDefName.c_str() ) );
		return nullptr;
	}

	Entity* newEntity = SpawnNewEntityOfType( *entityDef );
	newEntity->CreateZephyrScript( *entityDef );

	return newEntity;
}


//-----------------------------------------------------------------------------------------------
Entity* Map::SpawnNewEntityOfType( const EntityDefinition& entityDef )
{
	Entity* entity = new Entity( entityDef, this );
	m_entities.emplace_back( entity );
	return entity;
}


//-----------------------------------------------------------------------------------------------
void Map::UnloadAllEntityScripts()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->UnloadZephyrScript();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ReloadAllEntityScripts()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->ReloadZephyrScript();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::InitializeAllZephyrEntityVariables()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->InitializeZephyrEntityVariables();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::CallAllMapEntityZephyrSpawnEvents( Entity* player )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr
			 || entity == player )
		{
			continue;
		}

		entity->FireSpawnEvent();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::RemoveOwnershipOfEntity( Entity* entityToRemove )
{
	// TODO: Fire leave events for each collision entity
	entityToRemove->m_collidingEntities.clear();

	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		if ( entity == entityToRemove )
		{
			m_entities[entityIdx] = nullptr;
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::TakeOwnershipOfEntity( Entity* entityToAdd )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			entity = entityToAdd;
			return;
		}
	}

	m_entities.push_back( entityToAdd );
}


//-----------------------------------------------------------------------------------------------
Entity* Map::GetEntityById( EntityId id )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		if ( entity->GetId() == id )
		{
			return entity;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
Entity* Map::GetEntityByName( const std::string& name )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		if ( entity->GetName() == name )
		{
			return entity;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
Entity* Map::GetClosestEntityInSector( const Vec3& observerPos, float forwardDegrees, float apertureDegrees, float maxDist )
{
	float maxDistToSearch = maxDist;
	Entity* closestEntity = nullptr;

	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		// TODO: Do this with a raycast onto physics bounds
		if ( IsPointInForwardSector2D( entity->GetPosition().XY(), observerPos.XY(), forwardDegrees, apertureDegrees, maxDistToSearch ) )
		{
			closestEntity = entity;
			maxDistToSearch = GetDistance2D( observerPos.XY(), entity->GetPosition().XY() );
		}
	}

	return closestEntity;
}


//-----------------------------------------------------------------------------------------------
Entity* Map::GetEntityFromRaycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const
{
	RaycastResult result = Raycast( startPos, forwardNormal, maxDist );
	return result.impactEntity;
}


//-----------------------------------------------------------------------------------------------
void Map::LoadEntities( const std::vector<MapEntityDefinition>& mapEntityDefs )
{
	for ( int mapEntityIdx = 0; mapEntityIdx < (int)mapEntityDefs.size(); ++mapEntityIdx )
	{
		const MapEntityDefinition& mapEntityDef = mapEntityDefs[mapEntityIdx];
		if ( mapEntityDef.entityDef == nullptr )
		{
			continue;
		}

		Entity* newEntity = SpawnNewEntityOfType( *mapEntityDef.entityDef );
		if ( newEntity == nullptr )
		{
			continue;
		}

		// Must be saved before initializing zephyr script
		newEntity->SetName( mapEntityDef.name );
		m_world->SaveEntityByName( newEntity );

		newEntity->CreateZephyrScript( *mapEntityDef.entityDef );

		newEntity->SetPosition( mapEntityDef.position );
		newEntity->SetOrientationDegrees( mapEntityDef.yawDegrees );

		// Define initial script values defined in map file
		// Note: These will override any initial values already defined in the EntityDefinition
		newEntity->InitializeScriptValues( mapEntityDef.zephyrScriptInitialValues );
		newEntity->SetEntityVariableInitializers( mapEntityDef.zephyrEntityVarInits );

		newEntity->SetRigidbody( m_physicsScene->CreateCylinderRigidbody( mapEntityDef.position, (*mapEntityDef.entityDef).GetMass(), newEntity->GetPhysicsRadius() ));
		newEntity->m_rigidbody->SetSimulationMode( SIMULATION_MODE_DYNAMIC );

		for ( const ColliderData& colData : newEntity->GetColliderDataVec() )
		{
			switch ( colData.type )
			{
				case COLLIDER_SPHERE:
					newEntity->m_rigidbody->TakeCollider( m_physicsScene->CreateSphereCollider( colData.radius, colData.offsetFromCenter ) );
					break;

				case COLLIDER_OBB3:
					newEntity->m_rigidbody->TakeCollider( m_physicsScene->CreateOBB3Collider( colData.obb3, colData.offsetFromCenter ) );
					break;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::ResolveCollisionEvents( Entity* entity, Entity* otherEntity )
{
	EventArgs args;
	args.SetValue( "otherEntity", otherEntity->GetId() );
	args.SetValue( "otherEntityName", otherEntity->GetName() );
	args.SetValue( "otherEntityType", otherEntity->GetType() );

	EventArgs otherArgs;
	otherArgs.SetValue( "otherEntity", entity->GetId() );
	otherArgs.SetValue( "otherEntityName", entity->GetName() );
	otherArgs.SetValue( "otherEntityType", entity->GetType() );

	// TODO: 3D Physics?
	if ( DoDiscsOverlap( entity->GetPosition().XY(), entity->GetPhysicsRadius(), otherEntity->GetPosition().XY(), otherEntity->GetPhysicsRadius() ) )
	{
		// If entities are already colliding, fire stay
		if ( entity->m_collidingEntities.count( otherEntity->GetId() ) > 0 )
		{
			// Fire events as long as the entities are still valid
			if ( entity != nullptr )
			{
				entity->FireScriptEvent( "OnCollisionStay", &args );
			}
			if ( otherEntity != nullptr )
			{
				otherEntity->FireScriptEvent( "OnCollisionStay", &otherArgs );
			}
		}
		else
		{
			entity->m_collidingEntities.insert( otherEntity->GetId() );
			otherEntity->m_collidingEntities.insert( entity->GetId() );

			// Fire events as long as the entities are still valid
			if ( entity != nullptr )
			{
				entity->FireScriptEvent( "OnCollisionEnter", &args );
			}
			if ( otherEntity != nullptr )
			{
				otherEntity->FireScriptEvent( "OnCollisionEnter", &otherArgs );
			}
		}
	}
	else
	{
		// Entities were colliding earlier, fire leave events
		if ( entity != nullptr && otherEntity != nullptr
			 && entity->m_collidingEntities.count( otherEntity->GetId() ) > 0 )
		{
			entity->m_collidingEntities.erase( otherEntity->GetId() );
			otherEntity->m_collidingEntities.erase( entity->GetId() );

			// Fire events as long as the entities are still valid
			if ( entity != nullptr )
			{
				entity->FireScriptEvent( "OnCollisionLeave", &args );
			}
			if ( otherEntity != nullptr )
			{
				otherEntity->FireScriptEvent( "OnCollisionLeave", &otherArgs );
			}
		}
	}
}
