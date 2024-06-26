#pragma once
#include "Game/Tile.hpp"
#include "Game/GameEntity.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct MapData;
struct MapEntityDefinition;
struct PhysicsScene;
struct ZephyrScene;
class GameEntity;
class World;


//-----------------------------------------------------------------------------------------------
struct RaycastResult
{
	Vec3 startPos;
	Vec3 forwardNormal;
	float maxDist = 0.f;
	bool didImpact = false;
	Vec3 impactPos;
	GameEntity* impactEntity = nullptr;
	float impactFraction = 0.f;
	float impactDist = 0.f;
	Vec3 impactSurfaceNormal;
};


//-----------------------------------------------------------------------------------------------
class Map
{
public:
	Map( const MapData& mapData, World* world );
	virtual ~Map();

	virtual void		Load() = 0;
	virtual void		Unload() = 0;

	virtual void		Update( float deltaSeconds );
	virtual void		UpdateMeshes() = 0;
	virtual void		Render() const;
	virtual void		DebugRender() const;

	const std::string&	GetName() const												{ return m_name; }

	// Adding and removing entities from map
	virtual GameEntity*		SpawnNewEntityOfType( const std::string& entityDefName );
	virtual GameEntity*		SpawnNewEntityOfType( const EntityDefinition& entityDef );
	void					RemoveOwnershipOfEntity( GameEntity* entityToRemove );
	void					TakeOwnershipOfEntity( GameEntity* entityToAdd );

	// REFACTOR: Move to Zephyr specific object, or maybe pre init, post init, etc.
	void					UnloadAllEntityScripts();
	void					ReloadAllEntityScripts();
	void					InitializeAllZephyrEntityVariables();
	void					CallAllMapEntityZephyrSpawnEvents( GameEntity* player );

	// Entity queries
	GameEntity*				GetEntityById( EntityId id );
	GameEntity*				GetEntityByName( const std::string& name );
	GameEntity*				GetClosestEntityInSector( const Vec3& observerPos, float forwardDegrees, float apertureDegrees, float maxDist );
	GameEntity*				GetEntityFromRaycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const;

	EntityComponent*		GetZephyrComponentFromEntityId( const EntityId& id );

protected:
	void LoadEntities( const std::vector<MapEntityDefinition>& mapEntityDefs );

	void UpdateRigidbodyTransformsFromEntities();
	void UpdateEntityTransformsFromRigidbodies();

	virtual RaycastResult Raycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const = 0;

protected:
	std::string				m_name;
	World*					m_world = nullptr;
	
	PhysicsScene*			m_physicsScene = nullptr;
	ZephyrScene*			m_zephyrScene = nullptr;

	// Multiplayer TODO: Make this into an array
	// REFACTOR: Move to EntitySpawn object or something
	Vec3					m_playerStartPos = Vec3::ZERO;
	float					m_playerStartYaw = 0.f;

	std::vector<GameEntity*>			m_entities;
	// TODO: Change to actual object once my memory manager is in
};
