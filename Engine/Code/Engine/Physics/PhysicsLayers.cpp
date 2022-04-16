#include "Engine/Physics/PhysicsLayers.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Game/GameCommon.hpp"


//-----------------------------------------------------------------------------------------------
void PhysicsLayers::PopulateFromXml( XmlElement* root )
{
	// Initialize layers
	for ( int layerIdx = 0; layerIdx < 32; ++layerIdx )
	{
		m_layerInteractions[layerIdx] = ~0U;
	}

	m_layerToIndexMap.clear();

	// Parse layers
	XmlElement* layersElem = root->FirstChildElement( "Layers" );
	if ( layersElem != nullptr )
	{
		XmlElement* layerElem = layersElem->FirstChildElement( "Layer" );
		int layerNum = 0;
		while ( layerElem != nullptr )
		{
			if ( layerNum > 31 )
			{
				g_devConsole->PrintError( "Only a maximum of 32 physics layers can be defined, ignoring the rest" );
				break;
			}

			std::string layerName = ParseXmlAttribute( *layerElem, "name", "" );
			if ( layerName.empty() )
			{
				g_devConsole->PrintError( Stringf( "Layer %i is missing a name attribute", layerNum ) );
				return;
			}

			auto mapIter = m_layerToIndexMap.find( layerName );
			if ( mapIter != m_layerToIndexMap.end() )
			{
				g_devConsole->PrintError( Stringf( "Layer '%s' has already been defined in the layer list", layerName.c_str() ) );
				return;
			}

			// Save layer index into map for new layer
			m_layerToIndexMap[layerName] = layerNum;

			++layerNum;
			layerElem = layerElem->NextSiblingElement( "Layer" );
		}
	}

	// Parse interactions
	XmlElement* layerInteractionsElem = root->FirstChildElement( "LayerInteractions" );
	if ( layerInteractionsElem != nullptr )
	{
		XmlElement* layerInteractionElem = layerInteractionsElem->FirstChildElement();
		while ( layerInteractionElem != nullptr )
		{
			if ( IsEqualIgnoreCase( layerInteractionElem->Name(), "Disable" ) )
			{
				std::string layer1Name = ParseXmlAttribute( *layerInteractionElem, "layer1", "" );
				std::string layer2Name = ParseXmlAttribute( *layerInteractionElem, "layer2", "" );

				DisableLayerInteraction( layer1Name, layer2Name );
			}
			else if ( IsEqualIgnoreCase( layerInteractionElem->Name(), "Enable" ) )
			{
				std::string layer1Name = ParseXmlAttribute( *layerInteractionElem, "layer1", "" );
				std::string layer2Name = ParseXmlAttribute( *layerInteractionElem, "layer2", "" );

				EnableLayerInteraction( layer1Name, layer2Name );
			}
			else if ( IsEqualIgnoreCase( layerInteractionElem->Name(), "DisableAll" ) )
			{
				std::string layerName = ParseXmlAttribute( *layerInteractionElem, "layer", "" );

				DisableAllLayerInteraction( layerName );
			}
			else
			{
				g_devConsole->PrintError( Stringf( "Unknown LayerInteraction '%s' seen, ignoring", layerInteractionElem->Name() ) );
			}

			layerInteractionElem = layerInteractionsElem->NextSiblingElement();
		}
	}
}


//-----------------------------------------------------------------------------------------------
bool PhysicsLayers::DoLayersInteract( const std::string& layer0, const std::string& layer1 ) const
{
	int layer0Index = GetIndexForLayerName( layer0 );
	int layer1Index = GetIndexForLayerName( layer1 );

	if ( layer0Index == -1
		 || layer1Index == -1 )
	{
		return false;
	}

	return DoLayersInteract( layer0Index, layer1Index );
}


//-----------------------------------------------------------------------------------------------
void PhysicsLayers::EnableLayerInteraction( const std::string& layer0, const std::string& layer1 )
{
	int layer0Index = GetIndexForLayerName( layer0 );
	int layer1Index = GetIndexForLayerName( layer1 );

	if ( layer0Index == -1
		 || layer1Index == -1 )
	{
		return;
	}

	EnableLayerInteraction( layer0Index, layer1Index );
}


//-----------------------------------------------------------------------------------------------
void PhysicsLayers::DisableLayerInteraction( const std::string& layer0, const std::string& layer1 )
{
	int layer0Index = GetIndexForLayerName( layer0 );
	int layer1Index = GetIndexForLayerName( layer1 );

	if ( layer0Index == -1
		 || layer1Index == -1 )
	{
		return;
	}

	DisableLayerInteraction( layer0Index, layer1Index );
}


//-----------------------------------------------------------------------------------------------
void PhysicsLayers::DisableAllLayerInteraction( const std::string& layer )
{
	int layerIndex = GetIndexForLayerName( layer );

	if ( layerIndex == -1 )
	{
		return;
	}

	DisableAllLayerInteraction( layerIndex );
}


//-----------------------------------------------------------------------------------------------
bool PhysicsLayers::IsLayerDefined( const std::string& layer ) const
{
	return GetIndexForLayerName( layer ) != -1;
}


//-----------------------------------------------------------------------------------------------
bool PhysicsLayers::DoLayersInteract( uint layer0, uint layer1 ) const
{
	return ( m_layerInteractions[layer0] & ( 1 << layer1 ) ) != 0;
}


//-----------------------------------------------------------------------------------------------
void PhysicsLayers::EnableLayerInteraction( uint layer0, uint layer1 )
{
	m_layerInteractions[layer0] |= ( 1 << layer1 );
	m_layerInteractions[layer1] |= ( 1 << layer0 );
}


//-----------------------------------------------------------------------------------------------
void PhysicsLayers::DisableLayerInteraction( uint layer0, uint layer1 )
{
	m_layerInteractions[layer0] &= ~( 1 << layer1 );
	m_layerInteractions[layer1] &= ~( 1 << layer0 );
}


//-----------------------------------------------------------------------------------------------
void PhysicsLayers::DisableAllLayerInteraction( uint layer )
{
	m_layerInteractions[layer] = 0U;

	for ( int layerIdx = 0; layerIdx < 32; ++layerIdx )
	{
		m_layerInteractions[layerIdx] &= ~( 1 << layer );
	}
}


//-----------------------------------------------------------------------------------------------
int PhysicsLayers::GetIndexForLayerName( const std::string& layerName ) const
{
	// Errors will be reported by higher level functions

	auto mapIter = m_layerToIndexMap.find( layerName );
	if ( mapIter == m_layerToIndexMap.end() )
	{
		//g_devConsole->PrintError( Stringf( "Layer '%s' has not been defined in PhysicsConfig.xml", layerName.c_str() ) );
		return -1;
	}

	if ( mapIter->second > 31 )
	{
		//g_devConsole->PrintError( Stringf( "Layer '%s' was defined as index %i, but only 32 physics layers are allowed", layerName.c_str(), mapIter->second ) );
		return -1;
	}

	return mapIter->second;
}

