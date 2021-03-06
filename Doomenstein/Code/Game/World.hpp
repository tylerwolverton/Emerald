#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>
#include <map>
#include <unordered_map>


//-----------------------------------------------------------------------------------------------
struct Vec2;
struct Vec3;
class Clock;
class Entity;
class EntityDefinition;
class Map;
struct MapData;


//-----------------------------------------------------------------------------------------------
class World
{
	friend class LineMap;

public:
	World( Clock* gameClock );
	~World();

	void Update();
	void Render() const;
	void DebugRender() const;

	void AddNewMap( const MapData& mapData );
	void ChangeMap( const std::string& mapName );
	// TODO: Remove the need for these and make a SpawnEntity(mapName) instead?
	Map* GetMapByName( const std::string& name );
	Map* GetCurrentMap();

	Entity* GetClosestEntityInSector( const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist ) const;
	Entity* GetEntityFromRaycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const;

	void WarpEntityToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees );
	void WarpEntityToMap( Entity* entityToWarp, const std::string& destMapName, const Vec3& newPos, float newYawDegrees );
	bool IsMapLoaded( const std::string& mapName );

	// Entity access
	Entity* GetEntityById( EntityId id );
	void	RemoveEntityFromWorldById( EntityId id );
	Entity* GetEntityByIdInCurMap( EntityId id );
	Entity* GetEntityByName( const std::string& name );
	Entity* GetEntityByNameInCurMap( const std::string& name );
	void	SaveEntityByName( Entity* entity );
	// For WorldDef.xml
	void AddEntityFromDefinition( const EntityDefinition& entityDef, const std::string& entityName = "" );

	// Zephyr hot reloading
	void Reset();

	void UnloadAllEntityScripts();
	void ReloadAllEntityScripts();

	void InitializeAllZephyrEntityVariables();
	void CallAllZephyrSpawnEvents( Entity* player );

private:
	Map* GetLoadedMapByName( const std::string& mapName );

	void ClearMaps();
	void ClearEntities();

private:
	Map* m_curMap = nullptr;
	Clock* m_worldClock = nullptr;

	std::map<std::string, Map*> m_loadedMaps;

	// Entities saved in world
	std::vector<Entity*> m_worldEntities;
	std::unordered_map<std::string, Entity*> m_entitiesByName;
	std::unordered_map<EntityId, Entity*> m_entitiesById;
};