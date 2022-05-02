#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Physics/PhysicsLayers.hpp"
#include "Engine/Physics/Manifold.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class Collider;


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
extern PhysicsConfig* g_physicsConfig;


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
