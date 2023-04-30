#include "Game/Framework/World.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponent.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrScene.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"

#include "Game/Core/GameCommon.hpp"
#include "Game/Framework/Map.hpp"
#include "Game/Framework/GameEntity.hpp"
#include "Game/DataParsing/MapDefinition.hpp"
#include "Game/Graphics/SpriteAnimationComponent.hpp"
#include "Game/Graphics/SpriteAnimationScene.hpp"
#include "Game/Graphics/SpriteAnimationSystem.hpp"
#include "Game/Graphics/SpriteRenderingSystem.hpp"


//-----------------------------------------------------------------------------------------------
World::World( Clock* gameClock )
{
	m_worldClock = gameClock;
	m_zephyrScene = new ZephyrScene();
	m_spriteAnimScene = new SpriteAnimationScene();
}


//-----------------------------------------------------------------------------------------------
World::~World()
{
	PTR_SAFE_DELETE( m_zephyrScene );
	PTR_SAFE_DELETE( m_spriteAnimScene );
	PTR_MAP_SAFE_DELETE( m_loadedMaps );
}


//-----------------------------------------------------------------------------------------------
void World::Update()
{
	float deltaSeconds = (float)m_worldClock->GetLastDeltaSeconds();

	ZephyrSystem::UpdateScene( *m_zephyrScene );
	SpriteAnimationSystem::AdvanceAnimations( *m_spriteAnimScene, deltaSeconds );

	if ( m_curMap == nullptr )
	{
		return;
	}

	m_curMap->Update( deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
void World::Render() const
{
	if ( m_curMap != nullptr )
	{
		m_curMap->Render();
	}

	SpriteRenderingSystem::RenderScene( *m_spriteAnimScene, m_worldEntities );
}


//-----------------------------------------------------------------------------------------------
void World::DebugRender() const
{
	if ( m_curMap != nullptr )
	{
		m_curMap->DebugRender();
	}

	SpriteRenderingSystem::DebugRenderScene( *m_spriteAnimScene, m_worldEntities );
}


//-----------------------------------------------------------------------------------------------
void World::AddNewMap( const MapDefinition& mapDef )
{
	/*if ( mapData.type == "TileMap" )
	{
		TileMap* tileMap = new TileMap( mapData, this );
		m_loadedMaps[mapData.mapName] = tileMap;
	}*/

	Map* map = new Map( mapDef, this );
	m_loadedMaps[mapDef.mapName] = map;

	// Load entities after map has been fully created
	m_loadedMaps[mapDef.mapName]->LoadEntitiesFromInitialData( mapDef.mapEntitiesSpawnParams );
}


//-----------------------------------------------------------------------------------------------
void World::ChangeMap( const std::string& mapName, GameEntity* player )
{
	Map* newMap = GetLoadedMapByName( mapName );
	if ( newMap == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Map '%s' has not been loaded into world", mapName.c_str() ) );
		return;
	}

	if ( m_curMap != nullptr )
	{
		m_curMap->Unload();
	}

	m_curMap = newMap;
	m_curMap->Load( player );
	g_devConsole->PrintString( Stringf( "Map '%s' loaded", mapName.c_str() ), Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void World::Reset()
{
	UnloadAllEntityScripts();
	ClearMapsAndEntities();

	m_zephyrScene->Destroy();
	m_spriteAnimScene->Destroy();
}


//-----------------------------------------------------------------------------------------------
void World::UnloadAllEntityScripts()
{
	ZephyrSystem::UnloadZephyrScripts( *m_zephyrScene );

	for ( auto& map : m_loadedMaps )
	{
		map.second->UnloadAllEntityScripts();
	}
}


//-----------------------------------------------------------------------------------------------
void World::ReloadAllEntityScripts()
{
	ZephyrSystem::ReloadZephyrScripts( *m_zephyrScene );

	for ( auto& map : m_loadedMaps )
	{
		map.second->ReloadAllEntityScripts();
	}
}


//-----------------------------------------------------------------------------------------------
void World::ClearMapsAndEntities()
{
	// Note: Since Map owns its entities, Map destructor will delete entities in each map
	PTR_MAP_SAFE_DELETE( m_loadedMaps );

	m_curMap = nullptr;

	m_entitiesByName.clear();
	m_entitiesById.clear();

	PTR_VECTOR_SAFE_DELETE( m_worldEntities );
}


//-----------------------------------------------------------------------------------------------
void World::WarpEntityToMap( GameEntity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees )
{
	Map* destMap = GetLoadedMapByName( destMapName );

	// TODO: Verify portal target maps exist while loading xml files
	// Warp to a new map if one is specified and the entity is the player
	if ( destMap != nullptr
		 && destMap != m_curMap )
	{
		m_curMap->RemoveOwnershipOfEntity( entityToWarp );
		ChangeMap( destMapName, entityToWarp );
		// ChangeMap will transplant the player
		//m_curMap->TakeOwnershipOfEntity( entityToWarp );
	}

	entityToWarp->SetPosition( Vec3( newPos, 0.f ) );
	entityToWarp->SetOrientationDegrees( newYawDegrees );
}


//-----------------------------------------------------------------------------------------------
GameEntity* World::AddEntityFromDefinition( const EntityTypeDefinition& entityDef, const std::string& entityName )
{
	GameEntity* newEntity = new GameEntity( entityDef, nullptr );
	newEntity->SetName( entityName );

	m_worldEntities.push_back( newEntity );
	SaveEntityByName( newEntity );

	if ( entityDef.HasZephyrScript() )
	{
		m_zephyrScene->CreateAndAddComponent( newEntity, *entityDef.GetZephyrCompDef() );
	}
	if ( entityDef.HasSpriteAnimation() )
	{
		m_spriteAnimScene->CreateAndAddComponent( newEntity, *entityDef.GetSpriteAnimationCompDef() );
	}

	return newEntity;
}


//-----------------------------------------------------------------------------------------------
GameEntity* World::SpawnNewWorldEntity( const EntitySpawnParams& entitySpawnParams )
{
	SceneSpawnParams sceneSpawnParams;
	sceneSpawnParams.zephyrScene = m_zephyrScene;
	sceneSpawnParams.spriteAnimScene = m_spriteAnimScene;

	GameEntity* newEntity = SpawnNewEntity( entitySpawnParams, sceneSpawnParams, nullptr );

	m_worldEntities.push_back( newEntity );

	return newEntity;
}


//-----------------------------------------------------------------------------------------------
GameEntity* World::SpawnNewEntity( const EntitySpawnParams& entitySpawnParams, const SceneSpawnParams& sceneSpawnParams, Map* map )
{
	if ( entitySpawnParams.entityDef == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Tried to spawn an entity with null entityDef" ) );
		return nullptr;
	}

	GameEntity* newEntity = new GameEntity( *entitySpawnParams.entityDef, map );
	newEntity->SetName( entitySpawnParams.name );
	newEntity->SetPosition( entitySpawnParams.position );

	if ( !newEntity->GetName().empty() )
	{
		SaveEntityByName( newEntity );
	}
	else
	{
		SaveEntityById( newEntity );
	}

	CreateAndAttachEntityComponents( newEntity, entitySpawnParams, sceneSpawnParams );

	return newEntity;
}


//-----------------------------------------------------------------------------------------------
void World::CreateAndAttachEntityComponents( GameEntity* newEntity, const EntitySpawnParams& entitySpawnParams, const SceneSpawnParams& sceneSpawnParams )
{
	// ZephyrComponent
	if ( entitySpawnParams.entityDef->HasZephyrScript()
		 && sceneSpawnParams.zephyrScene != nullptr )
	{
		ZephyrComponent* zephyrComp = sceneSpawnParams.zephyrScene->CreateAndAddComponent( newEntity, *entitySpawnParams.entityDef->GetZephyrCompDef() );
		if ( zephyrComp != nullptr )
		{
			// Define initial script values defined in map file
			// Note: These will override any initial values already defined in the EntityTypeDefinition
			ZephyrSystem::InitializeGlobalVariables( zephyrComp, entitySpawnParams.zephyrScriptInitialValues );
			// TODO: This may be a bug, if map overwrites all will it remove something defined in the entity but not map?
			zephyrComp->SetEntityVariableInitializers( entitySpawnParams.zephyrEntityVarInits );
		}
	}

	// SpriteAnimationComponent
	if ( entitySpawnParams.entityDef->HasSpriteAnimation()
		 && sceneSpawnParams.spriteAnimScene != nullptr )
	{
		sceneSpawnParams.spriteAnimScene->CreateAndAddComponent( newEntity, *entitySpawnParams.entityDef->GetSpriteAnimationCompDef() );
	}
}


//-----------------------------------------------------------------------------------------------
void World::InitializeAllZephyrEntityVariables()
{
	ZephyrSystem::InitializeAllZephyrEntityVariables( *m_zephyrScene );

	for ( auto& map : m_loadedMaps )
	{
		map.second->InitializeAllZephyrEntityVariables();
	}
}


//-----------------------------------------------------------------------------------------------
void World::CallAllZephyrSpawnEvents()
{
	ZephyrSystem::FireAllSpawnEvents( *m_zephyrScene );

	for ( auto& map : m_loadedMaps )
	{
		map.second->CallAllMapEntityZephyrSpawnEvents();
	}
}


//-----------------------------------------------------------------------------------------------
bool World::IsMapLoaded( const std::string& mapName )
{
	return GetLoadedMapByName( mapName ) != nullptr;
}


//-----------------------------------------------------------------------------------------------
Map* World::GetMapByName( const std::string& name )
{
	return GetLoadedMapByName( name );
}


//-----------------------------------------------------------------------------------------------
Map* World::GetLoadedMapByName( const std::string& mapName )
{
	auto mapIter = m_loadedMaps.find( mapName );
	if ( mapIter == m_loadedMaps.end() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
Map* World::GetCurrentMap()
{
	return m_curMap;
}


//-----------------------------------------------------------------------------------------------
void World::SaveEntityById( GameEntity* entity )
{
	m_entitiesById[entity->GetId()] = entity;
}


//-----------------------------------------------------------------------------------------------
GameEntity* World::GetEntityById( EntityId id )
{
	auto mapIter = m_entitiesById.find( id );
	if ( mapIter != m_entitiesById.end() )
	{
		return mapIter->second;
	}

	// Look in this map first
	GameEntity* entity = GetEntityByIdInCurMap( id );
	if ( entity != nullptr )
	{
		return entity;
	}

	for ( auto& map : m_loadedMaps )
	{
		if ( map.second == m_curMap )
		{
			continue;
		}

		entity = map.second->GetEntityById( id );
		if ( entity != nullptr )
		{
			return entity;
		}
	}

	for ( auto& worldEntity : m_worldEntities )
	{
		if ( worldEntity != nullptr
			 && worldEntity->GetId() == id )
		{
			return worldEntity;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void World::RemoveEntityFromWorldById( EntityId id )
{
	GameEntity* entity = GetEntityById( id );
	if ( entity != nullptr )
	{
		std::string name = entity->GetName();
		
		m_entitiesById.erase( id );
		m_entitiesByName.erase( name );
	}
}


//-----------------------------------------------------------------------------------------------
GameEntity* World::GetEntityByIdInCurMap( EntityId id )
{
	if ( m_curMap == nullptr )
	{
		return nullptr;
	}

	return m_curMap->GetEntityById( id );
}


//-----------------------------------------------------------------------------------------------
GameEntity* World::GetEntityByName( const std::string& name )
{
	auto entityIter = m_entitiesByName.find( name );
	if ( entityIter != m_entitiesByName.end() )
	{
		return entityIter->second;
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
GameEntity* World::GetEntityByNameInCurMap( const std::string& name )
{
	if ( m_curMap == nullptr )
	{
		return nullptr;
	}

	return m_curMap->GetEntityByName( name );
}


//-----------------------------------------------------------------------------------------------
void World::SaveEntityByName( GameEntity* entity )
{
	if ( entity == nullptr 
		 || entity->GetName().empty() )
	{
		return;
	}

	auto entityIter = m_entitiesByName.find( entity->GetName() );
	if ( entityIter != m_entitiesByName.end() )
	{
		g_devConsole->PrintError( Stringf( "Tried to save an entity with name '%s' in map '%s', but an entity with that name was already defined in map '%s'", 
										   entity->GetName().c_str(), 
										   entity->GetMap() ? entity->GetMap()->GetName().c_str() : "none",
										   entityIter->second->GetMap() ? entityIter->second->GetMap()->GetName().c_str() : "none" ) );
		return;
	}

	m_entitiesByName[entity->GetName()] = entity;
	m_entitiesById[entity->GetId()] = entity;
}


//-----------------------------------------------------------------------------------------------
GameEntity* World::GetEntityAtPosition( const Vec3& position ) const
{
	return m_curMap->GetEntityAtPosition( position );
}


//-----------------------------------------------------------------------------------------------
EntityComponent* World::GetComponentFromEntityId( const EntityId& id, const EntityComponentTypeId& componentTypeId )
{
	switch ( componentTypeId )
	{
		case ENTITY_COMPONENT_TYPE_ZEPHYR:
		{
			return GetZephyrComponentFromEntityId( id );
		}
		break;

		case ENTITY_COMPONENT_TYPE_SPRITE_ANIM:
		{
			return GetSpriteAnimComponentFromEntityId( id );
		}
		break;

		default:
			return nullptr;
	}
}


//-----------------------------------------------------------------------------------------------
EntityComponent* World::GetZephyrComponentFromEntityId( const EntityId& id )
{
	// Check world entities
	for ( const auto& zephyrComponent : m_zephyrScene->zephyrComponents )
	{
		if ( zephyrComponent->GetParentEntityId() == id )
		{
			return zephyrComponent;
		}
	}

	// Check current map
	if ( m_curMap != nullptr )
	{
		EntityComponent* entityComponent = m_curMap->GetZephyrComponentFromEntityId( id );
		if ( entityComponent != nullptr )
		{
			return entityComponent;
		}
	}

	// Check the other maps
	for ( auto& map : m_loadedMaps )
	{
		if ( map.second == m_curMap
			 || map.second == nullptr )
		{
			continue;
		}

		EntityComponent* entityComponent = map.second->GetZephyrComponentFromEntityId( id );
		if ( entityComponent != nullptr )
		{
			return entityComponent;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
EntityComponent* World::GetSpriteAnimComponentFromEntityId( const EntityId& id )
{
	// Check world entities
	for ( const auto& spriteAnimComponent : m_spriteAnimScene->animComponents )
	{
		if ( spriteAnimComponent->GetParentEntityId() == id )
		{
			return spriteAnimComponent;
		}
	}
	
	// Check current map
	if ( m_curMap != nullptr )
	{
		EntityComponent* entityComponent = m_curMap->GetSpriteAnimComponentFromEntityId( id );
		if ( entityComponent != nullptr )
		{
			return entityComponent;
		}
	}

	// Check the other maps
	for ( auto& map : m_loadedMaps )
	{
		if ( map.second == m_curMap
			 || map.second == nullptr )
		{
			continue;
		}

		EntityComponent* entityComponent = map.second->GetSpriteAnimComponentFromEntityId( id );
		if ( entityComponent != nullptr )
		{
			return entityComponent;
		}
	}

	return nullptr;
}
