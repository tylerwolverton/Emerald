#pragma once
#include "Game/Tile.hpp"
#include "Game/Entity.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Entity;
//class CollisionResolver;
template <class CollisionPolicy>
class PhysicsScene;
class World;
struct MapData;
struct MapEntityDefinition;


//-----------------------------------------------------------------------------------------------
struct RaycastResult
{
	Vec3 startPos;
	Vec3 forwardNormal;
	float maxDist = 0.f;
	bool didImpact = false;
	Vec3 impactPos;
	Entity* impactEntity = nullptr;
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

	virtual Entity*		SpawnNewEntityOfType( const std::string& entityDefName );
	virtual Entity*		SpawnNewEntityOfType( const EntityDefinition& entityDef );

	void				UnloadAllEntityScripts();
	void				ReloadAllEntityScripts();
	void				InitializeAllZephyrEntityVariables();

	void				CallAllMapEntityZephyrSpawnEvents( Entity* player );

	void				RemoveOwnershipOfEntity( Entity* entityToRemove );
	void				TakeOwnershipOfEntity( Entity* entityToAdd );

	Entity*				GetEntityById( EntityId id );
	Entity*				GetEntityByName( const std::string& name );
	Entity*				GetClosestEntityInSector( const Vec3& observerPos, float forwardDegrees, float apertureDegrees, float maxDist );
	Entity*				GetEntityFromRaycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const;

protected:
	void LoadEntities( const std::vector<MapEntityDefinition>& mapEntityDefs );

	void ResolveCollisionEvents( Entity* entity, Entity* otherEntity );

	virtual RaycastResult Raycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const = 0;

protected:
	std::string							m_name;
	World*								m_world = nullptr;
	template <class CollisionPolicy>
	PhysicsScene<CollisionPolicy>*		m_physicsScene = nullptr;
	//CollisionResolver*		m_curCollisionResolver = nullptr;

	// Multiplayer TODO: Make this into an array
	Vec3					m_playerStartPos = Vec3::ZERO;
	float					m_playerStartYaw = 0.f;

	std::vector<Entity*>	m_entities;
};
