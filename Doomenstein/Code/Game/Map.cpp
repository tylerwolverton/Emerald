#include "Game/Map.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Framework/EntityComponent.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Physics/PhysicsScene.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponent.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrScene.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/GameEntity.hpp"
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
	m_zephyrScene = new ZephyrScene();

	LoadEntities( mapData.mapEntityDefs );
}


//-----------------------------------------------------------------------------------------------
Map::~Map()
{
	PTR_SAFE_DELETE( m_physicsScene );
	PTR_SAFE_DELETE( m_zephyrScene );
	PTR_VECTOR_SAFE_DELETE( m_entities );
}


//-----------------------------------------------------------------------------------------------
void Map::Update( float deltaSeconds )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->Update( deltaSeconds );
	}

	ZephyrSystem::UpdateScene( *m_zephyrScene );
	
	UpdateRigidbodyTransformsFromEntities();
	g_game->GetCurrentPhysicsSystem()->Update( *m_physicsScene );
	UpdateEntityTransformsFromRigidbodies();

	UpdateMeshes();
}


//-----------------------------------------------------------------------------------------------
void Map::Render() const
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity* const& entity = m_entities[entityIdx];
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
		GameEntity*const& entity = m_entities[entityIdx];
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
GameEntity* Map::SpawnNewEntityOfType( const std::string& entityDefName )
{
	EntityDefinition* entityDef = EntityDefinition::GetEntityDefinition( entityDefName );
	if ( entityDef == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Tried to spawn unrecognized entity '%s'", entityDefName.c_str() ) );
		return nullptr;
	}

	GameEntity* newEntity = SpawnNewEntityOfType( *entityDef );

	if ( entityDef->HasZephyrScript() )
	{
		m_zephyrScene->CreateAndAddComponent( newEntity, *entityDef->GetZephyrCompDef() );
	}

	return newEntity;
}


//-----------------------------------------------------------------------------------------------
GameEntity* Map::SpawnNewEntityOfType( const EntityDefinition& entityDef )
{
	GameEntity* entity = new GameEntity( entityDef, this );
	m_entities.emplace_back( entity );
	return entity;
}


//-----------------------------------------------------------------------------------------------
void Map::UnloadAllEntityScripts()
{
	ZephyrSystem::UnloadZephyrScripts( *m_zephyrScene );
}


//-----------------------------------------------------------------------------------------------
void Map::ReloadAllEntityScripts()
{
	ZephyrSystem::ReloadZephyrScripts( *m_zephyrScene );
}


//-----------------------------------------------------------------------------------------------
void Map::InitializeAllZephyrEntityVariables()
{
	ZephyrSystem::InitializeAllZephyrEntityVariables( *m_zephyrScene );
}


//-----------------------------------------------------------------------------------------------
void Map::CallAllMapEntityZephyrSpawnEvents( GameEntity* player )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity*& entity = m_entities[entityIdx];
		if ( entity == nullptr
			 || entity == player )
		{
			continue;
		}

		entity->FireSpawnEvent();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::RemoveOwnershipOfEntity( GameEntity* entityToRemove )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity*& entity = m_entities[entityIdx];
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
void Map::TakeOwnershipOfEntity( GameEntity* entityToAdd )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity*& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			entity = entityToAdd;
			return;
		}
	}

	m_entities.push_back( entityToAdd );
}


//-----------------------------------------------------------------------------------------------
GameEntity* Map::GetEntityById( EntityId id )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity*& entity = m_entities[entityIdx];
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
GameEntity* Map::GetEntityByName( const std::string& name )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity*& entity = m_entities[entityIdx];
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
GameEntity* Map::GetClosestEntityInSector( const Vec3& observerPos, float forwardDegrees, float apertureDegrees, float maxDist )
{
	float maxDistToSearch = maxDist;
	GameEntity* closestEntity = nullptr;

	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity*& entity = m_entities[entityIdx];
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
GameEntity* Map::GetEntityFromRaycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const
{
	RaycastResult result = Raycast( startPos, forwardNormal, maxDist );
	return result.impactEntity;
}


//-----------------------------------------------------------------------------------------------
EntityComponent* Map::GetZephyrComponentFromEntityId( const EntityId& id )
{
	for ( const auto& zephyrComponent : m_zephyrScene->zephyrComponents )
	{
		if ( zephyrComponent->GetParentEntityId() == id )
		{
			return zephyrComponent;
		}
	}

	return nullptr;
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

		GameEntity* newEntity = SpawnNewEntityOfType( *mapEntityDef.entityDef );
		if ( newEntity == nullptr )
		{
			continue;
		}

		// Must be saved before initializing zephyr script
		newEntity->SetName( mapEntityDef.name );
		m_world->SaveEntityByName( newEntity );

		if ( mapEntityDef.entityDef->HasZephyrScript() )
		{
			ZephyrComponent* zephyrComp = m_zephyrScene->CreateAndAddComponent( newEntity, *mapEntityDef.entityDef->GetZephyrCompDef() );

			newEntity->SetPosition( mapEntityDef.position );
			newEntity->SetOrientationDegrees( mapEntityDef.yawDegrees );

			if ( zephyrComp != nullptr )
			{
				// Define initial script values defined in map file
				// Note: These will override any initial values already defined in the EntityDefinition
				ZephyrSystem::InitializeGlobalVariables( zephyrComp, mapEntityDef.zephyrScriptInitialValues );
				// TODO: This may be a bug, if map overwrites all will it remove something defined in the entity but not map?
				zephyrComp->SetEntityVariableInitializers( mapEntityDef.zephyrEntityVarInits );
			}
		}

		if ( mapEntityDef.entityDef->HasPhysics() )
		{
			Rigidbody* rigidbody = m_physicsScene->CreateRigidbodyForEntity( newEntity->GetId() );
			rigidbody->SetLayer( mapEntityDef.entityDef->GetInitialCollisionLayer() );
			newEntity->SetRigidbody( rigidbody );
			rigidbody->SetPosition( mapEntityDef.position );
			rigidbody->SetMass( mapEntityDef.entityDef->GetMass() );
			newEntity->m_rigidbody->SetSimulationMode( SIMULATION_MODE_DYNAMIC );

			for ( const ColliderData& colData : newEntity->GetColliderDataVec() )
			{
				NamedProperties params;
				params.SetValue( "localPosition", colData.offsetFromCenter );

				switch ( colData.type )
				{
					case COLLIDER_SPHERE:
						params.SetValue( "radius", colData.radius );
						rigidbody->TakeCollider( m_physicsScene->CreateCollider( "sphere", &params ) );
						break;

					case COLLIDER_OBB3:
						params.SetValue( "obb3", colData.obb3 );
						rigidbody->TakeCollider( m_physicsScene->CreateCollider( "obb3", &params ) );
						break;
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void Map::UpdateRigidbodyTransformsFromEntities()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->CopyTransformToPhysicsComponent();
	}
}


//-----------------------------------------------------------------------------------------------
void Map::UpdateEntityTransformsFromRigidbodies()
{
	for ( int rigidbodyIdx = 0; rigidbodyIdx < (int)m_physicsScene->rigidbodies.size(); ++rigidbodyIdx )
	{
		Rigidbody*& rigidbody = m_physicsScene->rigidbodies[rigidbodyIdx];
		if ( rigidbody == nullptr 
			 || rigidbody->GetParentEntityId() == INVALID_ENTITY_ID )
		{
			continue;
		}

		GameEntity* entity = GetEntityById( rigidbody->GetParentEntityId() );
		if ( entity == nullptr )
		{
			g_devConsole->PrintWarning("Somehow a rigidbody doesn't have a valid entity");
			continue;
		}

		entity->SetPosition( rigidbody->GetWorldPosition() );
		entity->SetOrientationDegrees( rigidbody->GetOrientationDegrees() );
	}
}


//
////-----------------------------------------------------------------------------------------------
//void Map::ResolveCollisionEvents( Entity* entity, Entity* otherEntity )
//{
//	EventArgs args;
//	args.SetValue( "otherEntity", otherEntity->GetId() );
//	args.SetValue( "otherEntityName", otherEntity->GetName() );
//	args.SetValue( "otherEntityType", otherEntity->GetType() );
//
//	EventArgs otherArgs;
//	otherArgs.SetValue( "otherEntity", entity->GetId() );
//	otherArgs.SetValue( "otherEntityName", entity->GetName() );
//	otherArgs.SetValue( "otherEntityType", entity->GetType() );
//
//	// TODO: 3D Physics?
//	if ( DoDiscsOverlap( entity->GetPosition().XY(), entity->GetPhysicsRadius(), otherEntity->GetPosition().XY(), otherEntity->GetPhysicsRadius() ) )
//	{
//		// If entities are already colliding, fire stay
//		if ( entity->m_collidingEntities.count( otherEntity->GetId() ) > 0 )
//		{
//			// Fire events as long as the entities are still valid
//			if ( entity != nullptr )
//			{
//				entity->FireScriptEvent( "OnCollisionStay", &args );
//			}
//			if ( otherEntity != nullptr )
//			{
//				otherEntity->FireScriptEvent( "OnCollisionStay", &otherArgs );
//			}
//		}
//		else
//		{
//			entity->m_collidingEntities.insert( otherEntity->GetId() );
//			otherEntity->m_collidingEntities.insert( entity->GetId() );
//
//			// Fire events as long as the entities are still valid
//			if ( entity != nullptr )
//			{
//				entity->FireScriptEvent( "OnCollisionEnter", &args );
//			}
//			if ( otherEntity != nullptr )
//			{
//				otherEntity->FireScriptEvent( "OnCollisionEnter", &otherArgs );
//			}
//		}
//	}
//	else
//	{
//		// Entities were colliding earlier, fire leave events
//		if ( entity != nullptr && otherEntity != nullptr
//			 && entity->m_collidingEntities.count( otherEntity->GetId() ) > 0 )
//		{
//			entity->m_collidingEntities.erase( otherEntity->GetId() );
//			otherEntity->m_collidingEntities.erase( entity->GetId() );
//
//			// Fire events as long as the entities are still valid
//			if ( entity != nullptr )
//			{
//				entity->FireScriptEvent( "OnCollisionLeave", &args );
//			}
//			if ( otherEntity != nullptr )
//			{
//				otherEntity->FireScriptEvent( "OnCollisionLeave", &otherArgs );
//			}
//		}
//	}
//}
