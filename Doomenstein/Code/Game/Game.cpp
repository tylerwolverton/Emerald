#include "Game/Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/MatrixUtils.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/JobSystem.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/TextBox.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Physics/3D/OBB3Collider.hpp"
#include "Engine/Physics/3D/SphereCollider.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/GPUMesh.hpp"
#include "Engine/Renderer/Material.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/D3D11Common.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/UI/UIPanel.hpp"
#include "Engine/UI/UIText.hpp"
#include "Engine/UI/UISystem.hpp"
#include "Engine/ZephyrCore/ZephyrCompiler.hpp"
#include "Engine/ZephyrCore/ZephyrInterpreter.hpp"
#include "Engine/ZephyrCore/ZephyrBytecodeChunk.hpp"
#include "Engine/ZephyrCore/ZephyrScriptDefinition.hpp"
#include "Engine/ZephyrCore/ZephyrSystem.hpp"
#include "Engine/ZephyrCore/ZephyrUtils.hpp"

#include "Game/Entity.hpp"
#include "Game/EntityController.hpp"
#include "Game/GameJobs.hpp"
#include "Game/MapData.hpp"
#include "Game/MapRegionTypeDefinition.hpp"
#include "Game/MapMaterialTypeDefinition.hpp"
#include "Game/TileDefinition.hpp"
#include "Game/World.hpp"


//-----------------------------------------------------------------------------------------------
static float s_mouseSensitivityMultiplier = 1.f;
static Vec3 s_ambientLightColor = Vec3( 1.f, 1.f, 1.f );


//-----------------------------------------------------------------------------------------------
Game::Game()
{
} 


//-----------------------------------------------------------------------------------------------
Game::~Game()
{
}


//-----------------------------------------------------------------------------------------------
void Game::Startup()
{
	Transform::s_axisOrientation.m_axisYawPitchRollOrder = eAxisYawPitchRollOrder::ZYX;

	Transform::s_identityOrientation.PushTransform( Mat44::CreateZRotationDegrees( -90.f ) );
	Transform::s_identityOrientation.PushTransform( Mat44::CreateXRotationDegrees( 90.f ) );
	g_renderer->SetFrontFaceWindOrder( true );

	g_eventSystem->RegisterEvent( "set_mouse_sensitivity", "Usage: set_mouse_sensitivity multiplier=NUMBER. Set the multiplier for mouse sensitivity.", eUsageLocation::DEV_CONSOLE, SetMouseSensitivity );
	g_eventSystem->RegisterEvent( "light_set_ambient_color", "Usage: light_set_ambient_color color=r,g,b", eUsageLocation::DEV_CONSOLE, SetAmbientLightColor );
	g_eventSystem->RegisterMethodEvent( "warp", "Usage: warp <map=string> <pos=float,float> <yaw=float>", eUsageLocation::DEV_CONSOLE, this, &Game::WarpMapCommand );

	g_inputSystem->PushMouseOptions( CURSOR_RELATIVE, false, true );
		
	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();
	g_renderer->Setup( m_gameClock );

	m_world = new World( m_gameClock );

	EnableDebugRendering();

	InitializeCameras();
	
	m_playerController = new EntityController();

	m_uiSystem = new UISystem();
	m_uiSystem->Startup( g_window, g_renderer );

	m_physicsSystem = new PhysicsSystem3D();
	m_physicsSystem->Startup( m_gameClock );
	g_colliderFactory->RegisterCreator( "obb3", &OBB3Collider::Create );
	g_colliderFactory->RegisterCreator( "sphere", &SphereCollider::Create );

	g_physicsConfig->PopulateFromXml();
	LoadAssets();

	AddGunToUI();

	m_startingMapName = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_startingMapName );
	g_devConsole->PrintString( Stringf( "Loading starting map: %s", m_startingMapName.c_str() ) );
	m_world->ChangeMap( m_startingMapName );

	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeCameras()
{
	Vec2 windowDimensions = g_window->GetDimensions();

	m_uiCamera = new Camera();
	m_uiCamera->SetOutputSize( windowDimensions );
	m_uiCamera->SetType( eCameraType::UI );
	m_uiCamera->SetPosition( Vec3( windowDimensions * .5f, 0.f ) );
	m_uiCamera->SetProjectionOrthographic( windowDimensions.y );
}


//-----------------------------------------------------------------------------------------------
void Game::AddGunToUI()
{
	// Add gun sprite
	SpriteSheet* gunSprite = SpriteSheet::GetSpriteSheetByName( "ViewModels" );
	Vec2 uvsAtMins, uvsAtMaxs;
	gunSprite->GetSpriteUVs( uvsAtMins, uvsAtMaxs, IntVec2::ZERO );

	UIAlignedPositionData posData;
	posData.alignmentWithinParentElement = ALIGN_BOTTOM_CENTER;
	posData.fractionOfParentDimensions = Vec2( .5f, 1.f );

	SpriteDefinition spriteDef( *gunSprite, 0, uvsAtMins, uvsAtMaxs );
	m_uiSystem->GetUIElementByName( "Viewport" )->AddImage( posData, &spriteDef );
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, true, false );

	TileDefinition::s_definitions.clear();
	
	m_uiSystem->Shutdown();
	m_physicsSystem->Shutdown();

	// Clean up member variables
	PTR_SAFE_DELETE( m_playerController );
	PTR_SAFE_DELETE( m_world );
	PTR_SAFE_DELETE( m_gameClock );
	PTR_SAFE_DELETE( m_rng );
	PTR_SAFE_DELETE( m_uiCamera );
	PTR_SAFE_DELETE( m_uiSystem );
	PTR_SAFE_DELETE( m_physicsSystem );
}


//-----------------------------------------------------------------------------------------------
void Game::RestartGame()
{
	Shutdown();
	Startup();
}


//-----------------------------------------------------------------------------------------------
void Game::Update()
{
	if ( !g_devConsole->IsOpen() ) 
	{
		UpdateFromKeyboard();
	}

	m_playerController->UpdateRotation();

	m_uiSystem->Update();

	FreeAllLights();
	m_world->Update();

	m_playerController->UpdateTranslation();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard()
{	
	if ( g_devConsole->IsOpen() )
	{
		return;
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
	{
		m_isDebugRendering = !m_isDebugRendering;
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F2 ) )
	{
		g_raytraceFollowCamera = !g_raytraceFollowCamera;
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F3 ) )
	{
		if ( !m_playerController->IsPossessing() )
		{
			m_playerController->PossessNearestEntity( *m_world );
		}
		else
		{
			m_playerController->Unpossess();
		}
	}

	if ( g_inputSystem->WasKeyJustPressed( KEY_F4 ) )
	{
		g_renderer->ReloadShaders();
	}
	
	if ( g_inputSystem->ConsumeAllKeyPresses( KEY_F5 ) )
	{
		ReloadGame();
		LoadStartingMap( m_startingMapName );
		return;
	}

	//if ( g_inputSystem->ConsumeAllKeyPresses( KEY_F6 ) )
	//{
	//	ReloadScripts();
	//}
}


//-----------------------------------------------------------------------------------------------
void Game::LoadStartingMap( const std::string& mapName )
{
	m_world->InitializeAllZephyrEntityVariables();

	m_world->ChangeMap( mapName );

	if ( m_playerController->IsPossessing() )
	{
		m_playerController->GetPossessedEntity()->FireSpawnEvent();
	}

	// TODO: Double check how this works
	m_world->CallAllZephyrSpawnEvents( m_playerController->GetPossessedEntity() );
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{

}


//-----------------------------------------------------------------------------------------------
void Game::TranslateCameraFPS( const Vec3& relativeTranslation )
{
	Vec2 forwardVec = Vec2::MakeFromPolarDegrees( m_playerController->GetYawDegrees() );
	Vec2 rightVec = forwardVec.GetRotatedMinus90Degrees();

	Vec2 translationXY( relativeTranslation.x * forwardVec  
						+ relativeTranslation.y * rightVec );
	
	Vec3 absoluteTranslation( translationXY, relativeTranslation.z );

	m_playerController->GetCurrentWorldCamera()->Translate( absoluteTranslation );
}


//-----------------------------------------------------------------------------------------------
void Game::SetLightDirectionToCamera( Light& light )
{
	light.direction = m_playerController->GetForwardVector();
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	Texture* backbuffer = g_renderer->GetBackBuffer();
	Texture* colorTarget = g_renderer->AcquireRenderTargetMatching( backbuffer );

	m_playerController->GetCurrentWorldCamera()->SetColorTarget( 0, colorTarget );

	m_playerController->GetCurrentWorldCamera()->BeginCamera();

	g_renderer->SetDepthTest( eCompareFunc::COMPARISON_LESS_EQUAL, true );

	// Lighting
	g_renderer->SetAmbientLight( s_ambientLightColor, m_ambientIntensity );
	for ( int lightIdx = 0; lightIdx < MAX_LIGHTS; ++lightIdx )
	{
		g_renderer->EnableLight( lightIdx, m_lightPool[lightIdx] );
	}

	g_renderer->SetGamma( m_gamma );

	m_world->Render();
	
	m_playerController->GetCurrentWorldCamera()->EndCamera();

	// Copy rendered data to backbuffer and set on camera
	g_renderer->CopyTexture( backbuffer, colorTarget );
	m_playerController->GetCurrentWorldCamera()->SetColorTarget( backbuffer );

	g_renderer->ReleaseRenderTarget( colorTarget );

	// Debug rendering
	if ( m_isDebugRendering )
	{
		DebugRender();
	}

	m_playerController->GetCurrentWorldCamera()->DebugRenderWorld();

	m_uiCamera->SetColorTarget( 0, backbuffer );
	g_renderer->BeginCamera( *m_uiCamera );

	m_uiSystem->Render();
	if ( m_isDebugRendering )
	{
		m_uiSystem->DebugRender();
	}

	g_renderer->EndCamera( *m_uiCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::DebugRender() const
{
	RenderDebugUI();

	DebugAddWorldBasis( Mat44::IDENTITY, 0.f, DEBUG_RENDER_ALWAYS );

	Mat44 compassMatrix = Mat44::CreateTranslation3D( m_playerController->GetPosition() + .1f * m_playerController->GetForwardVector() );
	DebugAddWorldBasis( compassMatrix, .01f, 0.f, Rgba8::WHITE, Rgba8::WHITE, DEBUG_RENDER_ALWAYS );

	m_world->DebugRender();
}


//-----------------------------------------------------------------------------------------------
void Game::RenderDebugUI() const
{
	// Camera position and orientation
	Transform cameraTransform = m_playerController->GetTransform();

	std::string cameraOrientationStr = Stringf( "Yaw: %.2f, Pitch: %.2f, Roll: %.2f",
												cameraTransform.GetYawDegrees(),
												cameraTransform.GetPitchDegrees(),
												cameraTransform.GetRollDegrees() );

	std::string cameraPositionStr = Stringf( "xyz=( %.2f, %.2f, %.2f )",
											 cameraTransform.GetPosition().x,
											 cameraTransform.GetPosition().y,
											 cameraTransform.GetPosition().z );

	DebugAddScreenTextf( Vec4( 0.f, .97f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::YELLOW, 0.f,
						 "Camera - %s     %s",
						 cameraOrientationStr.c_str(),
						 cameraPositionStr.c_str() );

	// Basis text
	Mat44 cameraOrientationMatrix = cameraTransform.GetOrientationAsMatrix();
	DebugAddScreenTextf( Vec4( 0.f, .94f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::RED, 0.f,
						 "iBasis ( forward +x world east when identity  )  ( %.2f, %.2f, %.2f )",
						 cameraOrientationMatrix.GetIBasis3D().x,
						 cameraOrientationMatrix.GetIBasis3D().y,
						 cameraOrientationMatrix.GetIBasis3D().z );

	DebugAddScreenTextf( Vec4( 0.f, .91f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::GREEN, 0.f,
						 "jBasis ( left    +y world north when identity )  ( %.2f, %.2f, %.2f )",
						 cameraOrientationMatrix.GetJBasis3D().x,
						 cameraOrientationMatrix.GetJBasis3D().y,
						 cameraOrientationMatrix.GetJBasis3D().z );

	DebugAddScreenTextf( Vec4( 0.f, .88f, 0.f, 0.f ), Vec2::ZERO, 15.f, Rgba8::BLUE, 0.f,
						 "kBasis ( up      +z world up when identity    )  ( %.2f, %.2f, %.2f )",
						 cameraOrientationMatrix.GetKBasis3D().x,
						 cameraOrientationMatrix.GetKBasis3D().y,
						 cameraOrientationMatrix.GetKBasis3D().z );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets...", Rgba8::WHITE );

	SpriteSheet::CreateAndRegister( "ViewModels", *( g_renderer->CreateOrGetTextureFromFile( "Data/Images/ViewModelsSpriteSheet_8x8.png" ) ), IntVec2( 8, 8 ) );
	
	LoadSounds();
	LoadXmlUIElements();

	LoadAndCompileZephyrScripts();
	LoadXmlEntityTypes();
	LoadWorldDefinitionFromXml();
	LoadXmlMapMaterials();
	LoadXmlMapRegions();
	LoadXmlMaps();
	
	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadSounds()
{
	g_devConsole->PrintString( "Loading Audio..." );

	std::string folderPath( "Data/Audio" );

	Strings audioFiles = GetFileNamesInFolder( folderPath, "*.*" );
	for ( int soundIdx = 0; soundIdx < (int)audioFiles.size(); ++soundIdx )
	{
		std::string soundName = GetFileNameWithoutExtension( audioFiles[soundIdx] );
		std::string& soundNameWithExtension = audioFiles[soundIdx];

		std::string soundFullPath( folderPath );
		soundFullPath += "/";
		soundFullPath += soundNameWithExtension;

		m_loadedSoundIds[soundName] = g_audioSystem->CreateOrGetSound( soundFullPath );
	}

	g_devConsole->PrintString( "Audio Files Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadXmlUIElements()
{
	g_devConsole->PrintString( "Loading UI Elements..." );

	const char* filePath = "Data/UI/UI.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if (loadError != tinyxml2::XML_SUCCESS)
	{
		g_devConsole->PrintError( "UI.xml could not be opened" );
		return;
	}

	XmlElement* root = doc.RootElement();
	m_uiSystem->LoadUIElementsFromXML( *root );

	g_devConsole->PrintString( "UI Elements Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
// TODO: Move this logic into EntityDefinitions, do same for the rest of the Load functions 
void Game::LoadXmlEntityTypes()
{
	g_devConsole->PrintString( "Loading Entity Types..." );

	// TODO: Set this in game config
	const char* filePath = "Data/Definitions/EntityTypes.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( "EntityTypes.xml could not be opened" );
		return;
	}

	XmlElement* root = doc.RootElement();
	if ( strcmp( root->Name(), "EntityTypes" ) )
	{
		g_devConsole->PrintError( "EntityTypes.xml: Incorrect root node name, must be EntityTypes" );
		return;
	}

	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		if ( !strcmp( element->Name(), "Actor" )
			 || !strcmp( element->Name(), "Entity" ) 
			 || !strcmp( element->Name(), "Projectile" ) 
			 || !strcmp( element->Name(), "Portal" ) )
		{
			EntityDefinition* entityTypeDef = new EntityDefinition( *element );
			if ( entityTypeDef->IsValid() )
			{
				EntityDefinition::s_definitions[entityTypeDef->GetName()] = entityTypeDef;
			}
		}
		else
		{
			g_devConsole->PrintError( Stringf( "EntityTypes.xml: Unsupported node '%s'", element->Name() ) );
		}

		element = element->NextSiblingElement();
	}
	
	g_devConsole->PrintString( "Entity Types Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadXmlMapMaterials()
{
	g_devConsole->PrintString( "Loading Map Materials..." );

	const char* filePath = "Data/Definitions/MapMaterialTypes.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( "MapMaterialsTypes.xml could not be opened" );
		return;
	}

	XmlElement* root = doc.RootElement();
	if ( strcmp( root->Name(), "MapMaterialTypes" ) )
	{
		g_devConsole->PrintError( "MapMaterialsTypes.xml: Incorrect root node name, must be MapMaterialTypes" );
		return;
	}

	m_defaultMaterialStr = ParseXmlAttribute( *root, "default", "" );
	if ( m_defaultMaterialStr == "" )
	{
		g_devConsole->PrintError( "MapMaterialsTypes.xml: No default material attribute defined" );
		return;
	}

	XmlElement* materialsSheetElement = root->FirstChildElement( "MaterialsSheet" );
	if ( materialsSheetElement == nullptr )
	{
		g_devConsole->PrintError( "MapMaterialsTypes.xml: Must define at least one MaterialsSheet node." );
		return;
	}

	while ( materialsSheetElement )
	{
		bool materialsSheetHasError = false;
		std::string name = ParseXmlAttribute( *materialsSheetElement, "name", "" );
		if ( name == "" )
		{
			g_devConsole->PrintError( Stringf( "MapMaterialsTypes.xml: MaterialsSheet node is missing a name" ) );
			materialsSheetHasError = true;
		}
		IntVec2 layout = ParseXmlAttribute( *materialsSheetElement, "layout", IntVec2::ZERO );
		if ( layout == IntVec2::ZERO )
		{
			g_devConsole->PrintError( Stringf( "MapMaterialsTypes.xml: MaterialsSheet node '%s' is missing a layout", name.c_str() ) );
			materialsSheetHasError = true;
		}

		if ( materialsSheetHasError )
		{
			materialsSheetElement = materialsSheetElement->NextSiblingElement( "MaterialsSheet" );
			continue;
		}

		XmlElement* diffuseElement = materialsSheetElement->FirstChildElement( "Diffuse" );
		if ( diffuseElement == nullptr )
		{
			g_devConsole->PrintError( Stringf( "MapMaterialsTypes.xml: MaterialsSheet node '%s' is missing a diffuse node", name.c_str() ) );
			materialsSheetElement = materialsSheetElement->NextSiblingElement( "MaterialsSheet" );
			continue;
		}

		std::string imagePath = ParseXmlAttribute( *diffuseElement, "image", "" );
		if ( imagePath == "" )
		{
			g_devConsole->PrintError( Stringf( "MapMaterialsTypes.xml: MaterialsSheet node '%s' is missing an image", name.c_str() ) );
		}
		else
		{
			SpriteSheet::CreateAndRegister( name, *g_renderer->CreateOrGetTextureFromFile( imagePath.c_str() ), layout );
		}

		materialsSheetElement = materialsSheetElement->NextSiblingElement( "MaterialsSheet" );
	}

	if ( SpriteSheet::s_definitions.empty() )
	{
		g_devConsole->PrintError( "MapMaterialsTypes.xml: Must define at least one valid MaterialsSheet node." );
		return;
	}

	// Parse MaterialType nodes
	XmlElement* element = root->FirstChildElement( "MaterialType" );
	while ( element )
	{
		MapMaterialTypeDefinition* mapMaterialTypeDef = new MapMaterialTypeDefinition( *element );
		if ( mapMaterialTypeDef->IsValid() )
		{
			MapMaterialTypeDefinition::s_definitions[mapMaterialTypeDef->GetName()] = mapMaterialTypeDef;
		}

		element = element->NextSiblingElement( "MaterialType" );
	}

	if ( MapMaterialTypeDefinition::GetMapMaterialTypeDefinition( m_defaultMaterialStr ) == nullptr )
	{
		g_devConsole->PrintError( Stringf( "MapMaterialsTypes.xml: Default material '%s' was not defined as a MaterialType node.", m_defaultMaterialStr.c_str() ) );
		return;
	}

	g_devConsole->PrintString( "Map Materials Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadXmlMapRegions()
{
	g_devConsole->PrintString( "Loading Map Regions..." );
	
	const char* filePath = "Data/Definitions/MapRegionTypes.xml";

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( "MapRegionTypes.xml could not be opened" );
		return;
	}

	XmlElement* root = doc.RootElement();
	if ( strcmp( root->Name(), "MapRegionTypes" ) )
	{
		g_devConsole->PrintError( "MapRegionTypes.xml: Incorrect root node name, must be MapRegionTypes" );
		return;
	}

	m_defaultMapRegionStr = ParseXmlAttribute( *root, "default", "" );
	if ( m_defaultMapRegionStr.empty() )
	{
		g_devConsole->PrintError( "MapRegionTypes.xml: No default region attribute defined" );
		return;
	}

	m_defaultMapRegionCollisionLayerStr = ParseXmlAttribute(*root, "defaultCollisionLayer", "");
	if ( m_defaultMapRegionCollisionLayerStr.empty() )
	{
		g_devConsole->PrintError("MapRegionTypes.xml: No defaultCollisionLayer region attribute defined");
		return;
	}

	XmlElement* element = root->FirstChildElement();
	while ( element )
	{
		MapRegionTypeDefinition* mapRegionTypeDef = new MapRegionTypeDefinition( *element, m_defaultMaterialStr, m_defaultMapRegionCollisionLayerStr );
		if ( mapRegionTypeDef->IsValid() )
		{
			MapRegionTypeDefinition::s_definitions[mapRegionTypeDef->GetName()] = mapRegionTypeDef;
		}

		element = element->NextSiblingElement();
	}
	
	if ( MapRegionTypeDefinition::GetMapRegionTypeDefinition( m_defaultMapRegionStr ) == nullptr )
	{
		g_devConsole->PrintError( Stringf( "MapRegionTypes.xml: Default region '%s' was not defined as a RegionType node.", m_defaultMapRegionStr.c_str() ) );
		return;
	}

	g_devConsole->PrintString( "Map Regions Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadXmlMaps()
{
	g_devConsole->PrintString( "Loading Maps..." );

	std::string folderPath( "Data/Maps" );

	Strings mapFiles = GetFileNamesInFolder( folderPath, "*.xml" );
	for ( int mapIdx = 0; mapIdx < (int)mapFiles.size(); ++mapIdx )
	{
		std::string& mapName = mapFiles[mapIdx];

		std::string mapFullPath( folderPath );
		mapFullPath += "/";
		mapFullPath += mapName;

		XmlDocument doc;
		XmlError loadError = doc.LoadFile( mapFullPath.c_str() );
		if ( loadError != tinyxml2::XML_SUCCESS )
		{
			g_devConsole->PrintError( Stringf( "'%s' could not be opened", mapFullPath.c_str() ) );
			continue;
		}

		XmlElement* root = doc.RootElement();
		if ( strcmp( root->Name(), "MapDefinition" ) )
		{
			g_devConsole->PrintError( Stringf( "'%s': Incorrect root node name, must be MapDefinition", mapFullPath.c_str() ) );
			return;
		}

		MapData mapData( *root, GetFileNameWithoutExtension( mapName ), m_defaultMapRegionStr );

		if ( mapData.isValid )
		{
			m_world->AddNewMap( mapData );
		}
	}

	g_devConsole->PrintString( "Maps Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadWorldDefinitionFromXml()
{
	g_devConsole->PrintString( "Loading World Definition..." );

	std::string filePath = Stringf( "Data/Definitions/WorldDef.xml" );

	XmlDocument doc;
	XmlError loadError = doc.LoadFile( filePath.c_str() );
	if ( loadError != tinyxml2::XML_SUCCESS )
	{
		g_devConsole->PrintError( Stringf( "The world xml file '%s' could not be opened.", filePath.c_str() ) );
		return;
	}

	XmlElement* root = doc.RootElement();
	if ( strcmp( root->Name(), "WorldDefinition" ) )
	{
		g_devConsole->PrintError( Stringf( "'%s': Incorrect root node name, must be WorldDefinition", filePath.c_str() ) );
		return;
	}

	// Parse entities node
	XmlElement* entitiesElement = root->FirstChildElement( "Entities" );
	XmlElement* entityElement = entitiesElement->FirstChildElement();
	while ( entityElement )
	{
		if ( !strcmp( entityElement->Name(), "Actor" )
			 || !strcmp( entityElement->Name(), "Entity" )
			 || !strcmp( entityElement->Name(), "Projectile" )
			 || !strcmp( entityElement->Name(), "Portal" )
			 || !strcmp( entityElement->Name(), "Pickup" ) )
		{
			std::string entityTypeStr = ParseXmlAttribute( *entityElement, "type", "" );
			if ( entityTypeStr.empty() )
			{
				g_devConsole->PrintError( Stringf( "'%s': %s is missing a type attribute", filePath.c_str(), entityElement->Name() ) );
				return;
			}

			EntityDefinition* entityTypeDef = EntityDefinition::GetEntityDefinition( entityTypeStr );
			if ( entityTypeDef == nullptr )
			{
				g_devConsole->PrintError( Stringf( "'%s': Entity type '%s' was not defined in EntityTypes.xml", filePath.c_str(), entityTypeStr.c_str() ) );
				return;
			}

			std::string entityName = ParseXmlAttribute( *entityElement, "name", "" );

			m_world->AddEntityFromDefinition( *entityTypeDef, entityName );
		}
		else
		{
			g_devConsole->PrintError( Stringf( "WorldDef.xml: Unsupported node '%s'", entityElement->Name() ) );
		}

		entityElement = entityElement->NextSiblingElement();
	}

	g_devConsole->PrintString( "World Definition Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAndCompileZephyrScripts()
{
	g_devConsole->PrintString( "Loading Zephyr Scripts..." );

	std::string folderPath( "Data/Scripts" );

	Strings scriptFiles = GetFileNamesInFolder( folderPath, "*.zephyr" );
	for ( int scriptIdx = 0; scriptIdx < (int)scriptFiles.size(); ++scriptIdx )
	{
		std::string& scriptName = scriptFiles[scriptIdx];

		std::string scriptFullPath( folderPath );
		scriptFullPath += "/";
		scriptFullPath += scriptName;

		// Save compiled script into static map
		ZephyrScriptDefinition* scriptDef = ZephyrCompiler::CompileScriptFile( scriptFullPath );
		scriptDef->m_name = scriptName;

		ZephyrScriptDefinition::s_definitions[scriptFullPath] = scriptDef;
	}

	g_devConsole->PrintString( "Zephyr Scripts Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::ReloadGame()
{
	if ( m_curMusicId != (SoundPlaybackID)-1 )
	{
		g_audioSystem->StopSound( m_curMusicId );
	}

	m_playerController->Unpossess();
	m_world->Reset();
	g_zephyrSystem->StopAllTimers();

	g_gameConfigBlackboard.Clear();
	PopulateGameConfig();
	g_physicsConfig->PopulateFromXml();

	m_startingMapName = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_startingMapName );


	PTR_MAP_SAFE_DELETE( ZephyrScriptDefinition::s_definitions );
	PTR_MAP_SAFE_DELETE( EntityDefinition::s_definitions );
	PTR_MAP_SAFE_DELETE( MapMaterialTypeDefinition::s_definitions );
	PTR_MAP_SAFE_DELETE( TileDefinition::s_definitions );
	PTR_VECTOR_SAFE_DELETE( SpriteSheet::s_definitions );

	m_loadedSoundIds.clear();

	LoadSounds();
	LoadXmlUIElements();

	LoadAndCompileZephyrScripts();
	LoadXmlEntityTypes();
	LoadWorldDefinitionFromXml();
	LoadXmlMapMaterials();
	LoadXmlMapRegions();
	LoadXmlMaps();

	EventArgs args;
	g_eventSystem->FireEvent( "OnGameStart", &args );
	g_devConsole->PrintString( "Data files reloaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::ReloadScripts()
{
	m_world->UnloadAllEntityScripts();

	PTR_MAP_SAFE_DELETE( ZephyrScriptDefinition::s_definitions );

	LoadAndCompileZephyrScripts();

	for ( auto& entityDef : EntityDefinition::s_definitions )
	{
		if ( entityDef.second != nullptr )
		{
			entityDef.second->ReloadZephyrScriptDefinition();
		}
	}

	m_world->ReloadAllEntityScripts();

	g_devConsole->PrintString( "Scripts reloaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeMap( const std::string& mapName )
{
	m_world->ChangeMap( mapName );
}


//-----------------------------------------------------------------------------------------------
void Game::AddScreenShakeIntensity(float intensity)
{
	m_screenShakeIntensity += intensity;
}


//-----------------------------------------------------------------------------------------------
void Game::SetCameraPositionAndYaw( const Vec2& pos, float yaw )
{
	SetCameraPositionAndYaw( Vec3( pos, 0.5f ), yaw );
}


//-----------------------------------------------------------------------------------------------
void Game::SetCameraPositionAndYaw( const Vec3& pos, float yaw )
{
	Transform newTransform = m_playerController->GetTransform();
	newTransform.SetPosition( pos );
	newTransform.SetOrientationFromPitchRollYawDegrees( 0.f, 0.f, yaw );

	m_playerController->GetCurrentWorldCamera()->SetTransform( newTransform );
}


//-----------------------------------------------------------------------------------------------
void Game::WarpToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees )
{
	WarpToMap( entityToWarp, destMapName, Vec3( newPos, .5f ), newYawDegrees );
}


//-----------------------------------------------------------------------------------------------
void Game::WarpToMap( Entity* entityToWarp, const std::string& destMapName, const Vec3& newPos, float newYawDegrees )
{
	// No entity specified, just load the new map and set camera position and orientation
	if ( entityToWarp == nullptr )
	{
		if ( destMapName != "" )
		{
			m_world->ChangeMap( destMapName );
		}

		SetCameraPositionAndYaw( newPos, newYawDegrees );

		return;
	}

	m_world->WarpEntityToMap( entityToWarp, destMapName, newPos, newYawDegrees );
}


//-----------------------------------------------------------------------------------------------
void Game::WarpEntityToMap( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees )
{
	m_world->WarpEntityToMap( entityToWarp, destMapName, newPos, newYawDegrees );
}


//-----------------------------------------------------------------------------------------------
void Game::WarpEntityToMap( Entity* entityToWarp, const std::string& destMapName, const Vec3& newPos, float newYawDegrees )
{
	m_world->WarpEntityToMap( entityToWarp, destMapName, newPos, newYawDegrees );
}


//-----------------------------------------------------------------------------------------------
void Game::WarpMapCommand( EventArgs* args )
{
	std::string mapStr = args->GetValue( "map", "" );
	std::string posStr = args->GetValue( "pos", "" );
	std::string yawStr = args->GetValue( "yaw", "" );
	
	// Print usage and list of maps with no arguments
	if ( mapStr == ""
		 && posStr == ""
		 && yawStr == "" )
	{
		g_devConsole->PrintString( "Usage: warp <map=string> <pos=float,float> <yaw=float>" );

		std::string folderPath( "Data/Maps" );
		Strings mapFiles = GetFileNamesInFolder( folderPath, "*.xml" );

		for ( int mapIdx = 0; mapIdx < (int)mapFiles.size(); ++mapIdx )
		{
			g_devConsole->PrintString( GetFileNameWithoutExtension( mapFiles[mapIdx] ) );
		}

		return;
	}

	g_devConsole->Close();

	if ( mapStr != "")
	{
		m_world->ChangeMap( mapStr );
	}

	Vec2 newPos = args->GetValue( "pos", m_playerController->GetPosition().XY() );
	float newYawDegrees = args->GetValue( "yaw", m_playerController->GetYawDegrees() );

	SetCameraPositionAndYaw( newPos, newYawDegrees );
}


//-----------------------------------------------------------------------------------------------
Entity* Game::GetEntityById( EntityId id )
{
	return m_world->GetEntityById( id );
}


//-----------------------------------------------------------------------------------------------
Entity* Game::GetEntityByName( const std::string& name )
{
	return m_world->GetEntityByName( name );
}


//-----------------------------------------------------------------------------------------------
Map* Game::GetMapByName( const std::string& name )
{
	return m_world->GetMapByName( name );
}


//-----------------------------------------------------------------------------------------------
Map* Game::GetCurrentMap()
{
	if ( m_world == nullptr )
	{
		return nullptr;
	}

	return m_world->GetCurrentMap();
}


//-----------------------------------------------------------------------------------------------
void Game::SaveEntityByName( Entity* entity )
{
	m_world->SaveEntityByName( entity );
}


//-----------------------------------------------------------------------------------------------
Entity* Game::GetEntityFromCameraRaycast( float maxDist ) const
{
	return m_world->GetEntityFromRaycast( m_playerController->GetPosition(), m_playerController->GetForwardVector(), maxDist );
}


//-----------------------------------------------------------------------------------------------
void Game::PushCamera( const GameCameraSettings& gameCameraSettings )
{
	m_playerController->PushCamera( gameCameraSettings );
}


//-----------------------------------------------------------------------------------------------
void Game::PopCamera()
{
	m_playerController->PopCamera();
}


//-----------------------------------------------------------------------------------------------
void Game::PlaySoundByName( const std::string& soundName, bool isLooped, float volume, float balance, float speed, bool isPaused )
{
	auto iter = m_loadedSoundIds.find( soundName );
	if ( iter == m_loadedSoundIds.end() )
	{
		g_devConsole->PrintError( Stringf( "Cannot play unregistered sound, '%s", soundName.c_str() ) );
		return;
	}

	SoundID soundId = iter->second;

	g_audioSystem->PlaySound( soundId, isLooped, volume, balance, speed, isPaused );
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeMusic( const std::string& musicName, bool isLooped, float volume, float balance, float speed, bool isPaused )
{
	auto iter = m_loadedSoundIds.find( musicName );
	if ( iter == m_loadedSoundIds.end() )
	{
		g_devConsole->PrintError( Stringf( "Cannot play unregistered music, '%s", musicName.c_str() ) );
		return;
	}

	SoundID soundId = iter->second;
	if ( m_curMusicId != (SoundPlaybackID)-1 )
	{
		g_audioSystem->StopSound( m_curMusicId );
	}

	m_curMusicName = musicName;
	m_curMusicId = g_audioSystem->PlaySound( soundId, isLooped, volume, balance, speed, isPaused );
}


//-----------------------------------------------------------------------------------------------
int Game::AcquireAndSetLightFromPool( const Light& newLight )
{
	for ( int lightIdx = 0; lightIdx < MAX_LIGHTS; ++lightIdx )
	{
		if ( IsNearlyEqual( m_lightPool[lightIdx].intensity, 0.f ) )
		{
			m_lightPool[lightIdx] = newLight;
			return lightIdx;
		}
	}

	return -1;
}


//-----------------------------------------------------------------------------------------------
void Game::FreeLight( int lightIdx )
{
	if ( lightIdx >= 0 && lightIdx < MAX_LIGHTS )
	{
		m_lightPool[lightIdx] = Light();
	}
}


//-----------------------------------------------------------------------------------------------
void Game::FreeAllLights()
{
	for ( int lightIdx = 0; lightIdx < MAX_LIGHTS; ++lightIdx )
	{
		FreeLight( lightIdx );
	}
}


//-----------------------------------------------------------------------------------------------
bool Game::SetMouseSensitivity( EventArgs* args )
{
	s_mouseSensitivityMultiplier = args->GetValue( "multiplier", 1.f );

	return false;
}


//-----------------------------------------------------------------------------------------------
bool Game::SetAmbientLightColor( EventArgs* args )
{
	s_ambientLightColor = args->GetValue( "color", Vec3( 1.f, 1.f, 1.f ) );

	return false;
}


float Game::m_mouseSensitivityMultiplier;
