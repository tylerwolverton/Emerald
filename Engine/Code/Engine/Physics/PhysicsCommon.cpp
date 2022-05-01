#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"


//-----------------------------------------------------------------------------------------------
PhysicsConfig* g_physicsConfig = nullptr;


//-----------------------------------------------------------------------------------------------
void PhysicsConfig::PopulateFromXml()
{
	std::string path = g_gameConfigBlackboard.GetValue( "physicsConfigPath", "" );

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( path.c_str() );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( Stringf( "Could not open PhysicsConfig.xml at %s", path.c_str() ) );
		return;
	}

	XmlElement* root = doc.RootElement();

	// Parse root attributes

	m_layers.PopulateFromXml( root );
}


//-----------------------------------------------------------------------------------------------
bool IsPhysicsLayerDefined( const std::string& layer )
{
	return g_physicsConfig->GetLayers().IsLayerDefined( layer );
}


//-----------------------------------------------------------------------------------------------
uint GetPhysicsLayerFromName( const std::string& layer )
{
	return g_physicsConfig->GetLayers().GetIndexForLayerName( layer );
}


//-----------------------------------------------------------------------------------------------
bool DoPhysicsLayersInteract( const std::string& layer0, const std::string& layer1 )
{
	return g_physicsConfig->GetLayers().DoLayersInteract( layer0, layer1 );
}


//-----------------------------------------------------------------------------------------------
bool DoPhysicsLayersInteract( uint layer0, uint layer1 )
{
	return g_physicsConfig->GetLayers().DoLayersInteract( layer0, layer1 );
}


//-----------------------------------------------------------------------------------------------
void EnablePhysicsLayerInteraction( const std::string& layer0, const std::string& layer1 )
{
	g_physicsConfig->GetLayers().EnableLayerInteraction( layer0, layer1 );
}


//-----------------------------------------------------------------------------------------------
void EnablePhysicsLayerInteraction( uint layer0, uint layer1 )
{
	g_physicsConfig->GetLayers().EnableLayerInteraction( layer0, layer1 );
}


//-----------------------------------------------------------------------------------------------
void DisablePhysicsLayerInteraction( const std::string& layer0, const std::string& layer1 )
{
	g_physicsConfig->GetLayers().DisableLayerInteraction( layer0, layer1 );
}


//-----------------------------------------------------------------------------------------------
void DisablePhysicsLayerInteraction( uint layer0, uint layer1 )
{
	g_physicsConfig->GetLayers().DisableLayerInteraction( layer0, layer1 );
}


//-----------------------------------------------------------------------------------------------
void DisableAllPhysicsLayerInteraction( const std::string& layer )
{
	g_physicsConfig->GetLayers().DisableAllLayerInteraction( layer );
}


//-----------------------------------------------------------------------------------------------
void DisableAllPhysicsLayerInteraction( uint layer )
{
	g_physicsConfig->GetLayers().DisableAllLayerInteraction( layer );
}
