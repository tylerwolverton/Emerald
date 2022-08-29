#pragma once
#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class World;
struct MapDefinition;
struct MapEntityDefinition;


//-----------------------------------------------------------------------------------------------
class Map
{
	friend class Entity;
	friend class World;

public:
	Map( const MapDefinition& mapData, World* world );
	virtual ~Map();

	virtual void Load( Entity* player );
	virtual void Unload();

	virtual void Update( float deltaSeconds );
	//virtual void UpdateMesh() = 0;
	virtual void Render() const;
	virtual void DebugRender() const;

	virtual Entity* SpawnNewEntityOfType( const std::string& entityDefName );
	virtual Entity* SpawnNewEntityOfType( const EntityDefinition& entityDef );
	virtual Entity* SpawnNewEntityOfTypeAtPosition( const std::string& entityDefName, const Vec2& pos );
	virtual Entity* SpawnNewEntityOfTypeAtPosition( const std::string& entityDefName, const Vec3& pos );

	void			UnloadAllEntityScripts();
	void			ReloadAllEntityScripts();
	void			InitializeAllZephyrEntityVariables();

	void			CallAllMapEntityZephyrSpawnEvents( Entity* player );

	Vec3 GetPlayerStartPos() const										{ return m_playerStartPos; }
	std::string GetName() const											{ return m_name; }

	void RemoveOwnershipOfEntity( Entity* entityToRemove );
	void TakeOwnershipOfEntity( Entity* entityToAdd );

	Entity* GetEntityByName( const std::string& name );
	Entity* GetEntityById( EntityId id );

	Entity* GetEntityAtPosition( const Vec2& position );
	Entity* GetEntityAtPosition( const Vec3& position );

private:
	void LoadEntities( const std::vector<MapEntityDefinition>& mapEntityDefs );
	
	void AddToEntityList( Entity* entity );

	void DeleteDeadEntities();

protected:
	std::string					m_name;
	World*						m_world = nullptr;

	// Multiplayer TODO: Make this into an array
	Vec3						m_playerStartPos = Vec3::ZERO;
	float						m_playerStartYaw = 0.f;

	Entity*						m_player = nullptr;
	std::vector<Entity*>		m_entities;
};
