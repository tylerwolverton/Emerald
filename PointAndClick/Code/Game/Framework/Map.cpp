#include "Game/Framework/Map.hpp"
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

#include "Game/Core/GameCommon.hpp"
#include "Game/DataParsing/EntityTypeDefinition.hpp"
#include "Game/DataParsing/MapDefinition.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/GameEntity.hpp"
#include "Game/Graphics/SpriteAnimationScene.hpp"
#include "Game/Graphics/SpriteAnimationComponent.hpp"
#include "Game/Graphics/SpriteAnimationSystem.hpp"
#include "Game/Graphics/SpriteRenderingSystem.hpp"


//-----------------------------------------------------------------------------------------------
Map::Map( const MapDefinition& mapData, World* world )
	: m_name( mapData.mapName )
	, m_playerStartPos( mapData.playerStartPos )
	, m_world( world )
{
	m_zephyrScene = new ZephyrScene();
	m_spriteAnimScene = new SpriteAnimationScene();
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
	ZephyrSystem::UpdateScene( *m_zephyrScene );
	SpriteAnimationSystem::AdvanceAnimations( *m_spriteAnimScene, deltaSeconds );

	//UpdateMesh();

	DeleteGarbageEntities();
}


//-----------------------------------------------------------------------------------------------
void Map::Render() const
{
	SpriteRenderingSystem::RenderScene( *m_spriteAnimScene, m_entities );
}


//-----------------------------------------------------------------------------------------------
void Map::DebugRender() const
{
	SpriteRenderingSystem::DebugRenderScene( *m_spriteAnimScene, m_entities );
}


//-----------------------------------------------------------------------------------------------
GameEntity* Map::SpawnNewEntity( const EntitySpawnParams& entitySpawnParams )
{
	SceneSpawnParams sceneSpawnParams;
	sceneSpawnParams.zephyrScene = m_zephyrScene;
	sceneSpawnParams.spriteAnimScene = m_spriteAnimScene;

	GameEntity* newEntity = m_world->SpawnNewEntity( entitySpawnParams, sceneSpawnParams, this );

	AddToEntityList( newEntity );

	return newEntity;
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
	if ( entityToRemove == nullptr )
	{
		return;
	}

	RemoveFromEntityList( entityToRemove );
	entityToRemove->SetMap( nullptr );
}


//-----------------------------------------------------------------------------------------------
void Map::TakeOwnershipOfEntity( GameEntity* entityToAdd )
{
	if ( entityToAdd == nullptr )
	{
		return;
	}

	AddToEntityList( entityToAdd );
	entityToAdd->m_map = this;
}


//-----------------------------------------------------------------------------------------------
void Map::LoadEntitiesFromInitialData( const std::vector<EntitySpawnParams>& entitiesSpawnParams )
{
	for ( const EntitySpawnParams& spawnParams : entitiesSpawnParams )
	{
		SpawnNewEntity( spawnParams );
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
void Map::RemoveFromEntityList( GameEntity* entityToRemove )
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
void Map::DeleteGarbageEntities()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity*& entity = m_entities[entityIdx];
		if ( entity == nullptr
			 || !entity->IsGarbage() )
		{
			continue;
		}
		
		m_world->RemoveEntityFromWorldById( entity->GetId() );

		PTR_SAFE_DELETE( entity );

		m_entities[entityIdx] = nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
GameEntity* Map::GetEntityById( EntityId id )
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		GameEntity*& entity = m_entities[entityIdx];
		if ( entity == nullptr
			 || entity->IsGarbage() )
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
		if ( entity == nullptr
			 || entity->IsGarbage() )
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
GameEntity* Map::GetEntityAtPosition( const Vec2& position ) const
{
	for ( GameEntity* entity : m_entities )
	{
		// TODO: Interaction radius
		if ( IsPointInsideDiscFast( position, entity->GetPosition().XY(), entity->GetInteractionRadius() ) )
		{
			return entity;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
GameEntity* Map::GetEntityAtPosition( const Vec3& position ) const
{
	return GetEntityAtPosition( position.XY() );
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
EntityComponent* Map::GetSpriteAnimComponentFromEntityId( const EntityId& id )
{
	for ( const auto& spriteAnimComponent : m_spriteAnimScene->animComponents )
	{
		if ( spriteAnimComponent->GetParentEntityId() == id )
		{
			return spriteAnimComponent;
		}
	}

	return nullptr;
}
