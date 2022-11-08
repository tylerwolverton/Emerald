#include "Game/Map.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Physics/Rigidbody.hpp"
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
#include "Game/World.hpp"
#include "Game/GameEntity.hpp"
#include "Game/EntityDefinition.hpp"
#include "Game/MapDefinition.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( const MapDefinition& mapData, World* world )
	: m_name( mapData.mapName )
	, m_playerStartPos( mapData.playerStartPos )
	, m_playerStartYaw( mapData.playerStartYaw )
	, m_world( world )
{
	m_zephyrScene = new ZephyrScene();
}


//-----------------------------------------------------------------------------------------------
Map::~Map()
{
	PTR_SAFE_DELETE( m_zephyrScene );
	PTR_VECTOR_SAFE_DELETE( m_entities );
}


//-----------------------------------------------------------------------------------------------
void Map::Load( GameEntity* player )
{
	if ( player == nullptr )
	{
		g_devConsole->PrintWarning( Stringf( "Map '%s': Tried to load a map with a null player", m_name.c_str() ) );
		return;
	}

	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		if ( m_entities[entityIdx] != nullptr )
		{
			m_entities[entityIdx]->Load();
		}
	}
	
	m_player = player;
	//AddToEntityList( m_player );

	player->SetMap( this );
	player->SetPosition( m_playerStartPos );
	player->SetOrientationDegrees( m_playerStartYaw );
}


//-----------------------------------------------------------------------------------------------
void Map::Unload()
{
	// Remove reference to player from this map
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		if ( m_entities[entityIdx] == m_player )
		{
			m_entities[entityIdx] = nullptr;
			continue;
		}

		if ( m_entities[entityIdx] != nullptr )
		{
			m_entities[entityIdx]->Unload();
		}
	}

	m_player = nullptr;
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

	//UpdateMesh();

	DeleteDeadEntities();
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
		GameEntity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->DebugRender();
	}
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
	AddToEntityList( entity );
	return entity;
}


//-----------------------------------------------------------------------------------------------
GameEntity* Map::SpawnNewEntityOfTypeAtPosition( const std::string& entityDefName, const Vec2& pos )
{
	return SpawnNewEntityOfTypeAtPosition( entityDefName, Vec3( pos, 0.f ) );
}


//-----------------------------------------------------------------------------------------------
GameEntity* Map::SpawnNewEntityOfTypeAtPosition( const std::string& entityDefName, const Vec3& pos )
{
	GameEntity* entity = SpawnNewEntityOfType( entityDefName );
	entity->SetPosition( pos );

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
void Map::CallAllMapEntityZephyrSpawnEvents()
{
	ZephyrSystem::FireAllSpawnEvents( *m_zephyrScene );
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

	entityToAdd->m_map = this;
	m_entities.push_back( entityToAdd );
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
	}
}


//-----------------------------------------------------------------------------------------------
void Map::AddToEntityList( GameEntity* entity )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		if ( m_entities[entityIdx] == nullptr )
		{
			m_entities[entityIdx] = entity;
			return;
		}
	}

	m_entities.push_back( entity );
}


//-----------------------------------------------------------------------------------------------
void Map::DeleteDeadEntities()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity*& entity = m_entities[entityIdx];
		if ( entity == nullptr
			 || !entity->IsDead() )
		{
			continue;
		}
		
		m_world->RemoveEntityFromWorldById( entity->GetId() );

		PTR_SAFE_DELETE( entity );

		m_entities[entityIdx] = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
GameEntity* Map::GetEntityByName( const std::string& name )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity*& entity = m_entities[entityIdx];
		if ( entity == nullptr
			 || entity->IsDead() )
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
GameEntity* Map::GetEntityById( EntityId id )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity*& entity = m_entities[entityIdx];
		if ( entity == nullptr
			 || entity->IsDead() )
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
GameEntity* Map::GetEntityAtPosition( const Vec2& position )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity*& entity = m_entities[entityIdx];
		if ( entity == nullptr
			 || entity->IsDead() )
		{
			continue;
		}

		// TODO: Interaction radius
		if ( IsPointInsideDisc( position, entity->GetPosition().XY(), .5f ) )
		{
			return entity;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
GameEntity* Map::GetEntityAtPosition( const Vec3& position )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity*& entity = m_entities[entityIdx];
		if ( entity == nullptr
			 || entity->IsDead() )
		{
			continue;
		}

		// TODO: Interaction radius
		if ( IsPointInsideSphere( position, entity->GetPosition(), .5f ) )
		{
			return entity;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
GameEntity* Map::GetEntityAtPosition( const Vec3& position ) const
{
	for ( GameEntity* entity : m_entities )
	{
		if ( IsPointInsideDiscFast( position.XY(), entity->GetPosition().XY(), .5f ) )
		{
			return entity;
		}
	}

	return nullptr;
}

