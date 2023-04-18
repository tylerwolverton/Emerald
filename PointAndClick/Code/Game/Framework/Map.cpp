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
GameEntity* Map::SpawnNewEntityFromName( const std::string& entityDefName )
{
	EntityTypeDefinition* entityDef = EntityTypeDefinition::GetEntityDefinition( entityDefName );
	if ( entityDef == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Tried to spawn unrecognized entity '%s'", entityDefName.c_str() ) );
		return nullptr;
	}

	GameEntity* newEntity = SpawnNewEntityFromDef( *entityDef );
	if ( entityDef->HasZephyrScript() )
	{
		m_zephyrScene->CreateAndAddComponent( newEntity, *entityDef->GetZephyrCompDef() );
	}
	if ( entityDef->HasSpriteAnimation() )
	{
		m_spriteAnimScene->CreateAndAddComponent( newEntity, *entityDef->GetSpriteAnimationCompDef() );
	}

	return newEntity;
}


//-----------------------------------------------------------------------------------------------
GameEntity* Map::SpawnNewEntityFromDef( const EntityTypeDefinition& entityDef )
{
	GameEntity* entity = new GameEntity( entityDef, this );
	AddToEntityList( entity );
	return entity;
}


//-----------------------------------------------------------------------------------------------
GameEntity* Map::SpawnNewEntityFromNameAtPosition( const std::string& entityDefName, const Vec2& pos )
{
	return SpawnNewEntityFromNameAtPosition( entityDefName, Vec3( pos, 0.f ) );
}


//-----------------------------------------------------------------------------------------------
GameEntity* Map::SpawnNewEntityFromNameAtPosition( const std::string& entityDefName, const Vec3& pos )
{
	GameEntity* entity = SpawnNewEntityFromName( entityDefName );
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
void Map::LoadEntitiesFromInitialData( const std::vector<MapEntityDefinition>& mapEntityDefs )
{
	for ( int mapEntityIdx = 0; mapEntityIdx < (int)mapEntityDefs.size(); ++mapEntityIdx )
	{
		const MapEntityDefinition& mapEntityDef = mapEntityDefs[mapEntityIdx];
		if ( mapEntityDef.entityDef == nullptr )
		{
			continue;
		}

		GameEntity* newEntity = SpawnNewEntityFromDef( *mapEntityDef.entityDef );
		if ( newEntity == nullptr )
		{
			continue;
		}

		// Must be saved before initializing zephyr script
		newEntity->SetName( mapEntityDef.name );
		m_world->SaveEntityByName( newEntity );

		newEntity->SetPosition( mapEntityDef.position );

		CreateAndAttachEntityComponents( newEntity, mapEntityDef );
	}
}


//-----------------------------------------------------------------------------------------------
void Map::CreateAndAttachEntityComponents( GameEntity* newEntity, const MapEntityDefinition& mapEntityDef )
{
	// ZephyrComponent
	if ( mapEntityDef.entityDef->HasZephyrScript() )
	{
		ZephyrComponent* zephyrComp = m_zephyrScene->CreateAndAddComponent( newEntity, *mapEntityDef.entityDef->GetZephyrCompDef() );
		if ( zephyrComp != nullptr )
		{
			// Define initial script values defined in map file
			// Note: These will override any initial values already defined in the EntityTypeDefinition
			ZephyrSystem::InitializeGlobalVariables( zephyrComp, mapEntityDef.zephyrScriptInitialValues );
			// TODO: This may be a bug, if map overwrites all will it remove something defined in the entity but not map?
			zephyrComp->SetEntityVariableInitializers( mapEntityDef.zephyrEntityVarInits );
		}
	}

	// SpriteAnimationComponent
	if ( mapEntityDef.entityDef->HasSpriteAnimation() )
	{
		m_spriteAnimScene->CreateAndAddComponent( newEntity, *mapEntityDef.entityDef->GetSpriteAnimationCompDef() );
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
		if ( IsPointInsideDiscFast( position, entity->GetPosition().XY(), .5f ) )
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
