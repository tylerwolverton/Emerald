#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ObjectFactory.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Physics/PhysicsLayers.hpp"
#include "Engine/Physics/Manifold.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Collider;
class Rigidbody;
class NamedProperties;
template <class CollisionPolicy>
class PhysicsSystem;
class Simple3DCollision;


//-----------------------------------------------------------------------------------------------
class PhysicsConfig
{
public:
	void PopulateFromXml();

	PhysicsLayers GetLayers() const					{ return m_layers; }

private:
	PhysicsLayers m_layers;
};


//-----------------------------------------------------------------------------------------------
class Collision
{
public:
	IntVec2 id = IntVec2( -1, -1 );
	uint frameNum = 0;

	Collider* myCollider = nullptr;
	Collider* theirCollider = nullptr;
	Manifold collisionManifold;
};


//-----------------------------------------------------------------------------------------------
typedef void ( *AffectorFn )( Rigidbody* rigidbody );
// TODO: Change these from pointers to flat data
typedef std::vector<Rigidbody*> RigidbodyVector;
typedef std::vector<Collider*> ColliderVector;
typedef std::vector<Collision> CollisionVector;
typedef std::vector<AffectorFn> AffectorVector;

typedef PhysicsSystem<Simple3DCollision> PhysicsSystem3D;

typedef std::string ColliderId;
typedef NamedProperties ColliderParams;
typedef Collider* ( *ColliderCreationFunc )( ColliderParams* );
typedef ObjectFactory< Collider, ColliderId, ColliderCreationFunc, ColliderParams* > ColliderFactory;


//-----------------------------------------------------------------------------------------------
extern PhysicsConfig* g_physicsConfig;
extern ColliderFactory* g_colliderFactory;


//-----------------------------------------------------------------------------------------------
// Static physics layer methods
bool IsPhysicsLayerDefined( const std::string& layer );
uint GetPhysicsLayerFromName( const std::string& layer );

// Convenience "name based" functions
bool DoPhysicsLayersInteract( const std::string& layer0, const std::string& layer1 );
void EnablePhysicsLayerInteraction( const std::string& layer0, const std::string& layer1 );
void DisablePhysicsLayerInteraction( const std::string& layer0, const std::string& layer1 );
void DisableAllPhysicsLayerInteraction( const std::string& layer );

bool DoPhysicsLayersInteract( uint layer0, uint layer1 );
void EnablePhysicsLayerInteraction( uint layer0, uint layer1 );
void DisablePhysicsLayerInteraction( uint layer0, uint layer1 );
void DisableAllPhysicsLayerInteraction( uint layer );
