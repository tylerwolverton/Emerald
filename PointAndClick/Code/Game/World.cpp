#include "Game/World.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Time/Clock.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Map.hpp"
#include "Game/MapDefinition.hpp"


//-----------------------------------------------------------------------------------------------
World::World( Clock* gameClock )
{
	m_worldClock = gameClock;
}


//-----------------------------------------------------------------------------------------------
World::~World()
{
	PTR_MAP_SAFE_DELETE( m_loadedMaps );
}


//-----------------------------------------------------------------------------------------------
void World::Update()
{
	for ( Entity* entity : m_worldEntities )
	{
		if ( entity != nullptr )
		{
			entity->Update( (float)m_worldClock->GetLastDeltaSeconds() );
		}
	}

	if ( m_curMap == nullptr )
	{
		return;
	}

	m_curMap->Update( (float)m_worldClock->GetLastDeltaSeconds() );
}


//-----------------------------------------------------------------------------------------------
void World::Render() const
{
	for ( Entity* entity : m_worldEntities )
	{
		if ( entity != nullptr )
		{
			entity->Render();
		}
	}

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
	m_loadedMaps[mapDef.mapName]->LoadEntities( mapDef.mapEntityDefs );
}


//-----------------------------------------------------------------------------------------------
void World::ChangeMap( const std::string& mapName, Entity* player )
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
void World::WarpEntityToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees )
{
	Map* destMap = GetLoadedMapByName( destMapName );

	// TODO: Verify portal target maps exist while loading xml files
	// Warp to a new map if one is specified and the entity is the player
	if ( destMap != nullptr
		 && destMap != m_curMap
		 && entityToWarp->IsPlayer() )
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
bool World::IsMapLoaded( const std::string& mapName )
{
	return GetLoadedMapByName( mapName ) != nullptr;
}


//-----------------------------------------------------------------------------------------------
void World::Reset()
{
	UnloadAllEntityScripts();
	ClearMapsAndEntities();
}


//-----------------------------------------------------------------------------------------------
void World::UnloadAllEntityScripts()
{
	for ( auto& entity : m_worldEntities )
	{
		entity->UnloadZephyrScript();
	}

	for ( auto& map : m_loadedMaps )
	{
		map.second->UnloadAllEntityScripts();
	}
}


//-----------------------------------------------------------------------------------------------
void World::ReloadAllEntityScripts()
{
	for ( auto& entity : m_worldEntities )
	{
		entity->ReloadZephyrScript();
	}

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
void World::AddEntityFromDefinition( const EntityDefinition& entityDef, const std::string& entityName )
{
	Entity* newEntity = new Entity(entityDef, nullptr);
	newEntity->SetName( entityName );

	m_worldEntities.push_back( newEntity );
	SaveEntityByName( newEntity );

	newEntity->CreateZephyrScript( entityDef );
}


//-----------------------------------------------------------------------------------------------
Entity* World::GetEntityById( EntityId id )
{
	auto mapIter = m_entitiesById.find( id );
	if ( mapIter != m_entitiesById.end() )
	{
		return mapIter->second;
	}

	// Look in this map first
	Entity* entity = GetEntityByIdInCurMap( id );
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
	Entity* entity = GetEntityById( id );
	if ( entity != nullptr )
	{
		std::string name = entity->GetName();
		
		m_entitiesById.erase( id );
		m_entitiesByName.erase( name );
	}
}


//-----------------------------------------------------------------------------------------------
Entity* World::GetEntityByIdInCurMap( EntityId id )
{
	if ( m_curMap == nullptr )
	{
		return nullptr;
	}

	return m_curMap->GetEntityById( id );
}


//-----------------------------------------------------------------------------------------------
Entity* World::GetEntityByName( const std::string& name )
{
	auto entityIter = m_entitiesByName.find( name );
	if ( entityIter != m_entitiesByName.end() )
	{
		return entityIter->second;
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
Entity* World::GetEntityByNameInCurMap( const std::string& name )
{
	if ( m_curMap == nullptr )
	{
		return nullptr;
	}

	return m_curMap->GetEntityByName( name );
}


//-----------------------------------------------------------------------------------------------
void World::SaveEntityByName( Entity* entity )
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
void World::InitializeAllZephyrEntityVariables()
{
	for ( auto& entity : m_worldEntities )
	{
		entity->InitializeZephyrEntityVariables();
	}

	for ( auto& map : m_loadedMaps )
	{
		map.second->InitializeAllZephyrEntityVariables();
	}	
}


//-----------------------------------------------------------------------------------------------
void World::CallAllZephyrSpawnEvents( Entity* player )
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


