#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Game/Core/GameCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class GameEntity;
class EntityTypeDefinition;
class World;
struct MapDefinition;
struct MapEntityDefinition;
struct Vec2;
struct ZephyrScene;
struct SpriteAnimationScene;


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
	virtual GameEntity* SpawnNewEntityFromDef( const EntityTypeDefinition& entityDef );
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
	GameEntity* GetEntityAtPosition( const Vec2& position ) const;
	GameEntity* GetEntityAtPosition( const Vec3& position ) const;
	
	EntityComponent* GetZephyrComponentFromEntityId( const EntityId& id );
	EntityComponent* GetSpriteAnimComponentFromEntityId( const EntityId& id );

private:
	void LoadEntitiesFromInitialData( const std::vector<MapEntityDefinition>& mapEntityDefs );
	void CreateAndAttachEntityComponents( GameEntity* newEntity, const MapEntityDefinition& mapEntityDef );
	
	void AddToEntityList( GameEntity* entity );
	void RemoveFromEntityList( GameEntity* entity );

	void DeleteGarbageEntities();

protected:
	std::string					m_name;
	World*						m_world = nullptr;

	ZephyrScene*				m_zephyrScene = nullptr;
	SpriteAnimationScene*		m_spriteAnimScene = nullptr;

	// TODO: Remove and have portals say where in new map to go to?
	Vec3						m_playerStartPos = Vec3::ZERO;
	float						m_playerStartYaw = 0.f;

	GameEntity*					m_player = nullptr;
	std::vector<GameEntity*>	m_entities;
};