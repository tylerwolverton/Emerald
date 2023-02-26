#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Game/Core/GameCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class GameEntity;
class EntityDefinition;
class World;
struct MapDefinition;
struct MapEntityDefinition;
struct Vec2;
struct ZephyrScene;


//-----------------------------------------------------------------------------------------------
class Map
{
	friend class GameEntity;
	friend class World;

public:
	Map( const MapDefinition& mapData, World* world );
	virtual ~Map();

	virtual void Load( GameEntity* player );
	virtual void Unload();

	virtual void Update( float deltaSeconds );
	//virtual void UpdateMesh() = 0;
	virtual void Render() const;
	virtual void DebugRender() const;

	virtual GameEntity* SpawnNewEntityFromName( const std::string& entityDefName );
	virtual GameEntity* SpawnNewEntityFromDef( const EntityDefinition& entityDef );
	virtual GameEntity* SpawnNewEntityFromNameAtPosition( const std::string& entityDefName, const Vec2& pos );
	virtual GameEntity* SpawnNewEntityFromNameAtPosition( const std::string& entityDefName, const Vec3& pos );

	void			UnloadAllEntityScripts();
	void			ReloadAllEntityScripts();
	void			InitializeAllZephyrEntityVariables();
	void			CallAllMapEntityZephyrSpawnEvents();

	Vec3 GetPlayerStartPos() const										{ return m_playerStartPos; }
	std::string GetName() const											{ return m_name; }

	void RemoveOwnershipOfEntity( GameEntity* entityToRemove );
	void TakeOwnershipOfEntity( GameEntity* entityToAdd );

	GameEntity* GetEntityByName( const std::string& name );
	GameEntity* GetEntityById( EntityId id );
	EntityComponent* GetZephyrComponentFromEntityId( const EntityId& id );

	GameEntity* GetEntityAtPosition( const Vec2& position ) const;
	GameEntity* GetEntityAtPosition( const Vec3& position ) const;

private:
	void LoadEntitiesFromInitialData( const std::vector<MapEntityDefinition>& mapEntityDefs );
	
	void AddToEntityList( GameEntity* entity );
	void RemoveFromEntityList( GameEntity* entity );

	void DeleteGarbageEntities();

protected:
	std::string					m_name;
	World*						m_world = nullptr;

	ZephyrScene*				m_zephyrScene = nullptr;

	// TODO: Remove and have portals say where in new map to go to
	Vec3						m_playerStartPos = Vec3::ZERO;
	float						m_playerStartYaw = 0.f;

	GameEntity*					m_player = nullptr;
	std::vector<GameEntity*>	m_entities;
};
