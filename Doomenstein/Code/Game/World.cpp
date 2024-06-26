#include "Game/World.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Framework/EntityComponent.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponent.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrScene.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"

#include "Game/TileMap.hpp"
#include "Game/LineMap.hpp"
#include "Game/GameCommon.hpp"
#include "Game/MapData.hpp"


//-----------------------------------------------------------------------------------------------
World::World( Clock* gameClock )
{
	m_worldClock = new Clock( gameClock );
	m_zephyrScene = new ZephyrScene();
}


//-----------------------------------------------------------------------------------------------
World::~World()
{
	PTR_SAFE_DELETE( m_worldClock );

	std::map< std::string, Map* >::iterator it;

	for ( it = m_loadedMaps.begin(); it != m_loadedMaps.end(); it++ )
	{
		PTR_SAFE_DELETE( it->second );
	}

	PTR_SAFE_DELETE( m_zephyrScene );

	m_loadedMaps.clear();
}


//-----------------------------------------------------------------------------------------------
void World::Update()
{
	ZephyrSystem::UpdateScene( *m_zephyrScene );

	if ( m_curMap == nullptr )
	{
		return;
	}

	m_curMap->Update( (float)m_worldClock->GetLastDeltaSeconds() );
}


//-----------------------------------------------------------------------------------------------
void World::Render() const
{
	if ( m_curMap == nullptr )
	{
		return;
	}

	m_curMap->Render();
}


//-----------------------------------------------------------------------------------------------
void World::DebugRender() const
{
	if ( m_curMap == nullptr )
	{
		return;
	}

	m_curMap->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void World::AddNewMap( const MapData& mapData )
{
	if ( mapData.type == "TileMap" )
	{
		TileMap* tileMap = new TileMap( mapData, this );
		m_loadedMaps[mapData.mapName] = tileMap;
	}
	else if ( mapData.type == "LineMap" )
	{
		LineMap* lineMap = new LineMap( mapData, this );
		m_loadedMaps[mapData.mapName] = lineMap;
	}
}


//-----------------------------------------------------------------------------------------------
void World::ChangeMap( const std::string& mapName )
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
	
	if ( m_curMap != nullptr )
	{
		m_curMap->Load();
		g_devConsole->PrintString( Stringf( "Map '%s' loaded", mapName.c_str() ), Rgba8::GREEN );
	}
}


//-----------------------------------------------------------------------------------------------
Map* World::GetMapByName( const std::string& name )
{
	return GetLoadedMapByName( name );
}


//-----------------------------------------------------------------------------------------------
Map* World::GetCurrentMap()
{
	return m_curMap;
}


//-----------------------------------------------------------------------------------------------
GameEntity* World::GetClosestEntityInSector( const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist ) const
{
	if ( m_curMap != nullptr )
	{
		// TODO: 3D
		return m_curMap->GetClosestEntityInSector( Vec3( observerPos, 0.f ), forwardDegrees, apertureDegrees, maxDist );
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
GameEntity* World::GetEntityFromRaycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const
{
	return m_curMap->GetEntityFromRaycast( startPos, forwardNormal, maxDist );
}


//-----------------------------------------------------------------------------------------------
void World::WarpEntityToMap( GameEntity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees )
{
	WarpEntityToMap( entityToWarp, destMapName, Vec3( newPos, 0.f ), newYawDegrees );
}


//-----------------------------------------------------------------------------------------------
void World::WarpEntityToMap( GameEntity* entityToWarp, const std::string& destMapName, const Vec3& newPos, float newYawDegrees )
{
	if ( entityToWarp == nullptr )
	{
		g_devConsole->PrintWarning( "Tried to warp null entity" );
		return;
	}

	Map* destMap = GetLoadedMapByName( destMapName );

	// TODO: Verify portal target maps exist while loading xml files
	// Warp to a new map if one is specified and the entity is the player
	if ( destMap != nullptr
		 && destMap != m_curMap
		 && entityToWarp->IsPossessed() )
	{
		m_curMap->RemoveOwnershipOfEntity( entityToWarp );
		ChangeMap( destMapName );
		m_curMap->TakeOwnershipOfEntity( entityToWarp );
	}

	entityToWarp->SetPosition( newPos );
	entityToWarp->SetOrientationDegrees( newYawDegrees );
}


//-----------------------------------------------------------------------------------------------
bool World::IsMapLoaded( const std::string& mapName )
{
	return GetLoadedMapByName( mapName ) != nullptr;
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
										   entity->GetMap()->GetName().c_str(),
										   entityIter->second->GetMap()->GetName().c_str() ) );
		return;
	}

	m_entitiesByName[entity->GetName()] = entity;
	m_entitiesById[entity->GetId()] = entity;
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
void World::AddEntityFromDefinition( const EntityDefinition& entityDef, const std::string& entityName )
{
	GameEntity* newEntity = new GameEntity( entityDef, nullptr );

	newEntity->SetName( entityName );

	m_worldEntities.push_back( newEntity );
	SaveEntityByName( newEntity );

	if ( entityDef.HasZephyrScript() )
	{
		m_zephyrScene->CreateAndAddComponent( newEntity, *entityDef.GetZephyrCompDef() );
	}
}


//-----------------------------------------------------------------------------------------------
void World::Reset()
{
	UnloadAllEntityScripts();
	ClearEntities();
	ClearMaps();
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
void World::ClearMaps()
{
	PTR_MAP_SAFE_DELETE( m_loadedMaps );

	m_curMap = nullptr;
}


//-----------------------------------------------------------------------------------------------
void World::ClearEntities()
{
	m_entitiesByName.clear();

	PTR_VECTOR_SAFE_DELETE( m_worldEntities );
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
void World::CallAllZephyrSpawnEvents( GameEntity* player )
{
	for ( auto& entity : m_worldEntities )
	{
		entity->FireSpawnEvent();
	}

	for ( auto& map : m_loadedMaps )
	{
		map.second->CallAllMapEntityZephyrSpawnEvents( player );
	}
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
