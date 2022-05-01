#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/PhysicsLayers.hpp"

#include <string>

//-----------------------------------------------------------------------------------------------
class PhysicsConfig
{
public:
	void PopulateFromXml();

	PhysicsLayers GetLayers() const					{ return m_layers; }
public:
	PhysicsLayers m_layers;
};


//-----------------------------------------------------------------------------------------------
extern PhysicsConfig* g_physicsConfig;


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
