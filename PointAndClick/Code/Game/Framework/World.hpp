#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>
#include <map>
#include <unordered_map>


//-----------------------------------------------------------------------------------------------
struct MapDefinition;
struct Vec2;
struct Vec3;
struct ZephyrScene;
struct SpriteAnimationScene;
class Clock;
class EntityComponent;
class EntityTypeDefinition;
class GameEntity;
class Map;


//-----------------------------------------------------------------------------------------------
class World
{
public:
	World( Clock* gameClock );
	~World();

	void				Update();
	void				Render() const;
	void				DebugRender() const;

	void				AddNewMap( const MapDefinition& mapDef );
	void				ChangeMap( const std::string& mapName, GameEntity* player );

	// Zephyr hot reloading
	void				Reset();

	void				UnloadAllEntityScripts();
	void				ReloadAllEntityScripts();

	void				WarpEntityToMap( GameEntity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees );
	GameEntity*			AddEntityFromDefinition( const EntityTypeDefinition& entityDef, const std::string& entityName = "" );

	void				InitializeAllZephyrEntityVariables();
	void				CallAllZephyrSpawnEvents();

	bool				IsMapLoaded( const std::string& mapName );
	Map*				GetMapByName( const std::string& name );
	Map*				GetCurrentMap();

	GameEntity*			GetEntityById( EntityId id );
	void				RemoveEntityFromWorldById( EntityId id );
	GameEntity*			GetEntityByIdInCurMap( EntityId id );
	GameEntity*			GetEntityByName( const std::string& name );
	void				SaveEntityByName( GameEntity* entity );
	GameEntity*			GetEntityByNameInCurMap( const std::string& name );
	GameEntity*			GetEntityAtPosition( const Vec3& position ) const;

	// World switches on component type instead of map since it can see all world entities and each map
	EntityComponent*	GetComponentFromEntityId( const EntityId& id, const EntityComponentTypeId& componentTypeId );
	EntityComponent*	GetZephyrComponentFromEntityId( const EntityId& id );
	EntityComponent*	GetSpriteAnimComponentFromEntityId( const EntityId& id );

private:
	Map* GetLoadedMapByName( const std::string& mapName );

	void ClearMapsAndEntities();

private:
	Map* m_curMap = nullptr;
	Clock* m_worldClock = nullptr;

	std::map<std::string, Map*> m_loadedMaps;

	std::vector<GameEntity*> m_worldEntities;
	std::unordered_map<std::string, GameEntity*> m_entitiesByName;
	std::unordered_map<EntityId, GameEntity*> m_entitiesById;

	ZephyrScene* m_zephyrScene = nullptr;
	SpriteAnimationScene* m_spriteAnimScene = nullptr;
};
