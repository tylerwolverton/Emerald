#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>
#include <map>
#include <unordered_map>


//-----------------------------------------------------------------------------------------------
struct Vec2;
struct Vec3;
struct MapData;
struct ZephyrScene;
class Clock;
class GameEntity;
class EntityDefinition;
class Map;


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

	GameEntity* GetClosestEntityInSector( const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist ) const;
	GameEntity* GetEntityFromRaycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const;

	void WarpEntityToMap( GameEntity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees );
	void WarpEntityToMap( GameEntity* entityToWarp, const std::string& destMapName, const Vec3& newPos, float newYawDegrees );
	bool IsMapLoaded( const std::string& mapName );

	// Entity access
	GameEntity* GetEntityById( EntityId id );
	void	RemoveEntityFromWorldById( EntityId id );
	GameEntity* GetEntityByIdInCurMap( EntityId id );
	GameEntity* GetEntityByName( const std::string& name );
	GameEntity* GetEntityByNameInCurMap( const std::string& name );
	void	SaveEntityByName( GameEntity* entity );
	// For WorldDef.xml
	void AddEntityFromDefinition( const EntityDefinition& entityDef, const std::string& entityName = "" );

	// Zephyr hot reloading
	void Reset();

	void UnloadAllEntityScripts();
	void ReloadAllEntityScripts();

	void InitializeAllZephyrEntityVariables();
	void CallAllZephyrSpawnEvents( GameEntity* player );

private:
	Map* GetLoadedMapByName( const std::string& mapName );

	void ClearMaps();
	void ClearEntities();

private:
	Map* m_curMap = nullptr;
	Clock* m_worldClock = nullptr;

	std::map<std::string, Map*> m_loadedMaps;

	// Entities saved in world
	std::vector<GameEntity*> m_worldEntities;
	std::unordered_map<std::string, GameEntity*> m_entitiesByName;
	std::unordered_map<EntityId, GameEntity*> m_entitiesById;

	ZephyrScene* m_zephyrScene = nullptr;
};