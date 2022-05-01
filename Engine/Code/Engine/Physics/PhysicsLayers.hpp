#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/XmlUtils.hpp"

#include <map>
#include <string>


//-----------------------------------------------------------------------------------------------
class PhysicsLayers
{
public:
	void PopulateFromXml( XmlElement* root );

	bool IsLayerDefined( const std::string& layer ) const;

	// Convenience "name based" functions
	bool DoLayersInteract( const std::string& layer0, const std::string& layer1 ) const;
	void EnableLayerInteraction( const std::string& layer0, const std::string& layer1 );
	void DisableLayerInteraction( const std::string& layer0, const std::string& layer1 );
	void DisableAllLayerInteraction( const std::string& layer );

	bool DoLayersInteract( uint layer0, uint layer1 ) const;
	void EnableLayerInteraction( uint layer0, uint layer1 );
	void DisableLayerInteraction( uint layer0, uint layer1 );
	void DisableAllLayerInteraction( uint layer );

	int GetIndexForLayerName( const std::string& layerName ) const;

private:
	uint m_layerInteractions[32];

	std::map<std::string, int> m_layerToIndexMap;
};
