#include "Game/Framework/Game.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/TextBox.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/UI/UIPanel.hpp"
#include "Engine/Zephyr/Core/ZephyrCompiler.hpp"
#include "Engine/Zephyr/Core/ZephyrBytecodeChunk.hpp"
#include "Engine/Zephyr/Core/ZephyrScriptDefinition.hpp"
#include "Engine/Zephyr/Core/ZephyrUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponentDefinition.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"

#include "Game/DataParsing/EntityTypesParser.hpp"
#include "Game/DataParsing/WorldDefinitionParser.hpp"
#include "Game/DataParsing/MapDefinition.hpp"
#include "Game/Framework/World.hpp"
#include "Game/Framework/GameEntity.hpp"


//-----------------------------------------------------------------------------------------------
void Game::Startup()
{
	InitializeCameras();

	EnableDebugRendering();
	
	m_debugInfoTextBox = new TextBox( *g_renderer, AABB2( Vec2::ZERO, Vec2( 200.f, 80.f ) ) );

	m_rng = new RandomNumberGenerator();

	m_gameClock = new Clock();
	//m_gameClock->SetFrameLimits( 1.0 / 60.0, .1 );
	Clock::GetMaster()->SetFrameLimits( 1.0/60.0, .1 );

	g_renderer->Setup( m_gameClock );

	g_inputSystem->PushMouseOptions( CURSOR_ABSOLUTE, false, false );

	InitializeUI();

	m_world = new World( m_gameClock );

	m_startingMapName = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_startingMapName );
	
	g_eventSystem->RegisterMethodEvent( "print_bytecode_chunk", "Usage: print_bytecode_chunk entityName=<> chunkName=<>", eUsageLocation::DEV_CONSOLE, this, &Game::PrintBytecodeChunk );
	g_eventSystem->RegisterMethodEvent( "get_component_from_entity_id", "", eUsageLocation::GAME, this, &Game::GetComponentFromEntityId );

	g_devConsole->PrintString( "Game Started", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::Shutdown()
{
	m_uiSystem->Shutdown();

	// Clean up member variables
	PTR_SAFE_DELETE( m_world );
	PTR_SAFE_DELETE( m_rng );
	PTR_SAFE_DELETE( m_debugInfoTextBox );
	PTR_SAFE_DELETE( m_uiCamera );
	PTR_SAFE_DELETE( m_worldCamera );
	PTR_SAFE_DELETE( m_uiSystem );
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
	switch ( m_gameState )
	{
		case eGameState::LOADING:
		{
			switch ( m_loadingFrameNum )
			{
				case 0:
				{
					++m_loadingFrameNum;

					SoundID anticipation = g_audioSystem->CreateOrGetSound( "Data/Audio/Anticipation.mp3" );
					g_audioSystem->PlaySound( anticipation, false, .1f );
				}
				break;

				case 1:
				{
					LoadAssets();
					InitializePlayerController();
					ChangeGameState( eGameState::ATTRACT );
					Update();
				}
				break;
			}
		}
		break;

		case eGameState::ATTRACT:
		case eGameState::PAUSED:
		case eGameState::VICTORY:
		{
			UpdateFromKeyboard();
		}
		break;

		case eGameState::PLAYING:
		case eGameState::DIALOGUE:
		{
			UpdateFromKeyboard();

			m_playerController->UpdateFromKeyboard();
			
			m_world->Update();
		}
		break;
	}

	UpdateCameras();
	UpdateMousePositions();

	m_uiSystem->Update();
}


//-----------------------------------------------------------------------------------------------
void Game::Render() const
{
	g_renderer->BeginCamera( *m_worldCamera );

	switch ( m_gameState )
	{
		case eGameState::PLAYING:
		case eGameState::DIALOGUE:
		case eGameState::CUTSCENE:
		case eGameState::PAUSED:
		{
			m_world->Render();
			if ( m_isDebugRendering )
			{
				m_world->DebugRender();
			}
		}
		break;
	}

	g_renderer->EndCamera( *m_worldCamera );

	g_renderer->BeginCamera( *m_uiCamera );

	switch ( m_gameState )
	{
		// TODO: Make proper loading screen system
		case eGameState::LOADING:
		{
			std::vector<Vertex_PCU> vertexes;
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 450.f, 500.f ), 100.f, "Loading..." );

			g_renderer->BindTexture( 0, g_renderer->GetSystemFont()->GetTexture() );
			g_renderer->DrawVertexArray( vertexes );
		}
		break;

		case eGameState::ATTRACT:
		{
			std::vector<Vertex_PCU> vertexes;
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 450.f, 500.f ), 100.f, g_gameConfigBlackboard.GetValue( "windowTitle", "" ) );
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 500.f, 400.f ), 30.f, "Esc to Quit" );
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 500.f, 350.f ), 30.f, "Any Other Key to Start" );

			g_renderer->BindTexture( 0, g_renderer->GetSystemFont()->GetTexture() );
			g_renderer->DrawVertexArray( vertexes );
		}
		break;	
		
		case eGameState::PAUSED:
		{
			std::vector<Vertex_PCU> vertexes;
			DrawAABB2( g_renderer, AABB2( Vec2::ZERO, Vec2( WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS ) ), Rgba8( 0, 0, 0, 100 ) );

			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 500.f, 500.f ), 100.f, "Paused" );
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 550.f, 400.f ), 30.f, "Esc to Quit" );
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 550.f, 350.f ), 30.f, "Any Other Key to Resume" );

			g_renderer->BindTexture( 0, g_renderer->GetSystemFont()->GetTexture() );
			g_renderer->DrawVertexArray( vertexes );
		}
		break;

		case eGameState::VICTORY:
		{
			std::vector<Vertex_PCU> vertexes;
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 500.f, 500.f ), 100.f, "Victory" );
			g_renderer->GetSystemFont()->AppendVertsForText2D( vertexes, Vec2( 550.f, 400.f ), 30.f, "Press Enter to Return to Menu" );

			g_renderer->BindTexture( 0, g_renderer->GetSystemFont()->GetTexture() );
			g_renderer->DrawVertexArray( vertexes );
		}
		break;
	}

	m_uiSystem->Render();
	if ( m_isDebugRendering )
	{
		m_uiSystem->DebugRender();
	}

	g_renderer->EndCamera( *m_uiCamera );

	DebugRenderWorldToCamera( m_worldCamera );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeCameras()
{
	m_worldCamera = new Camera();
	m_worldCamera->SetOutputSize( Vec2( WINDOW_WIDTH, WINDOW_HEIGHT ) );
	m_worldCamera->SetClearMode( CLEAR_COLOR_BIT, Rgba8::BLACK );
	m_worldCamera->SetPosition( m_focalPoint );

	Vec2 windowDimensions = g_window->GetDimensions();

	m_uiCamera = new Camera();
	m_uiCamera->SetOutputSize( windowDimensions );
	m_uiCamera->SetPosition( Vec3( windowDimensions * .5f, 0.f ) );
	m_uiCamera->SetProjectionOrthographic( windowDimensions.y );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializePlayerController()
{
	std::string playerEntityName = g_gameConfigBlackboard.GetValue( std::string( "playerEntityName" ), "" );
	if ( playerEntityName.empty() )
	{
		g_devConsole->PrintError( "GameConfig.xml doesn't define a playerEntityName" );
		return;
	}

	EntityTypeDefinition* playerDef = EntityTypeDefinition::GetEntityDefinition( playerEntityName );
	if ( playerDef == nullptr )
	{
		g_devConsole->PrintError( "GameConfig.xml's playerEntityName was not loaded from EntityTypes.xml" );
		return;
	}

	GameEntity* playerEntity = m_world->AddEntityFromDefinition( *playerDef, playerEntityName );

	m_playerController = m_world->GetEntityByName( "PlayerController" );
	if ( m_playerController == nullptr )
	{
		g_devConsole->PrintError( "No PlayerController defined in world" );
		return;
	}

	EventArgs args;
	args.SetValue( "newPossessedEntity", playerEntity->GetId() );
	m_playerController->FireScriptEvent( "OnPossess", &args );
}


//-----------------------------------------------------------------------------------------------
void Game::InitializeUI()
{
	m_uiSystem = new UISystem();
	m_uiSystem->Startup( g_window, g_renderer );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAssets()
{
	g_devConsole->PrintString( "Loading Assets..." );

	// Music
	LoadSounds();
	LoadAndCompileZephyrScripts();
	LoadEntityTypes();
	LoadWorldDefinition();
	LoadMaps();

	g_devConsole->PrintString( "Assets Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadSounds()
{
	g_devConsole->PrintString( "Loading Audio..." );

	// TODO: Allow for subfolders
	std::string folderRoot( g_gameConfigBlackboard.GetValue( "audioRoot", "" ));

	Strings audioFiles = GetFileNamesInFolder( folderRoot, "*.*" );
	for ( int soundIdx = 0; soundIdx < (int)audioFiles.size(); ++soundIdx )
	{
		std::string soundName = GetFileNameWithoutExtension( audioFiles[soundIdx] );
		std::string& soundNameWithExtension = audioFiles[soundIdx];

		std::string soundFullPath( folderRoot );
		soundFullPath += "/";
		soundFullPath += soundNameWithExtension;

		m_loadedSoundIds[soundName] = g_audioSystem->CreateOrGetSound( soundFullPath );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::LoadMaps()
{
	g_devConsole->PrintString( "Loading Maps..." );

	std::string folderRoot( g_gameConfigBlackboard.GetValue( "mapsRoot", "" ) );

	Strings mapFiles = GetFileNamesInFolder( folderRoot, "*.*" );
	for ( int mapIdx = 0; mapIdx < (int)mapFiles.size(); ++mapIdx )
	{
		std::string& mapName = mapFiles[mapIdx];

		std::string mapFullPath( folderRoot );
		mapFullPath += "/";
		mapFullPath += mapName;

		MapDefinition mapData( mapFullPath );
		if ( mapData.isValid )
		{
			m_world->AddNewMap( mapData );
		}
	}

	g_devConsole->PrintString( "Maps Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadEntityTypes()
{
	g_devConsole->PrintString( "Loading Entity Types..." );

	std::string entityTypesPath( g_gameConfigBlackboard.GetValue( "entityTypesPath", "" ) );

	EntityTypesParser::ParseFromFile( entityTypesPath );

	g_devConsole->PrintString( "Entity Types Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadWorldDefinition()
{
	g_devConsole->PrintString( "Loading World Definition..." );

	std::string filePath( g_gameConfigBlackboard.GetValue( "worldDefPath", "" ) );

	std::map<std::string, EntityTypeDefinition*> worldEntities = WorldDefinitionParser::ParseEntitiesFromFile( filePath );
	for ( const auto& worldEntity : worldEntities )
	{
		m_world->AddEntityFromDefinition( *worldEntity.second, worldEntity.first );
	}

	g_devConsole->PrintString( "World Definition Loaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadAndCompileZephyrScripts()
{
	g_devConsole->PrintString( "Loading Zephyr Scripts..." );

	std::string folderRoot( g_gameConfigBlackboard.GetValue( "scriptsRoot", "" ) );

	Strings scriptFiles = GetFileNamesInFolder( folderRoot, "*.zephyr" );
	for ( int scriptIdx = 0; scriptIdx < (int)scriptFiles.size(); ++scriptIdx )
	{
		std::string& scriptName = scriptFiles[scriptIdx];

		std::string scriptFullPath( folderRoot );
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
	if ( m_gameState == eGameState::DIALOGUE 
		 || m_gameState == eGameState::CUTSCENE )
	{
		m_gameState = eGameState::PLAYING;
	}

	if ( m_curMusicId != (SoundPlaybackID)-1 )
	{
		g_audioSystem->StopSound( m_curMusicId );
	}

	m_world->Reset();

	g_zephyrSubsystem->StopAllTimers();

	g_gameConfigBlackboard.Clear();
	PopulateGameConfig();
	g_physicsConfig->PopulateFromXml();

	m_startingMapName = g_gameConfigBlackboard.GetValue( std::string( "startMap" ), m_startingMapName );

	PTR_MAP_SAFE_DELETE( ZephyrScriptDefinition::s_definitions );
	PTR_MAP_SAFE_DELETE( EntityTypeDefinition::s_definitions );
	PTR_VECTOR_SAFE_DELETE( SpriteSheet::s_definitions );

	//g_physicsSystem->Reset();
	m_loadedSoundIds.clear();

	LoadAssets();

	InitializePlayerController();

	EventArgs args;
	g_eventSystem->FireEvent( "OnGameStart", &args );
}


//-----------------------------------------------------------------------------------------------
void Game::ReloadScripts()
{
	m_world->UnloadAllEntityScripts();

	PTR_MAP_SAFE_DELETE( ZephyrScriptDefinition::s_definitions );
	
	LoadAndCompileZephyrScripts();

	for ( auto& entityDef : EntityTypeDefinition::s_definitions )
	{
		if ( entityDef.second != nullptr )
		{
			entityDef.second->GetZephyrCompDef()->ReloadZephyrScriptDefinition();
		}
	}

	m_world->ReloadAllEntityScripts();

	g_devConsole->PrintString( "Scripts reloaded", Rgba8::GREEN );
}


//-----------------------------------------------------------------------------------------------
void Game::LoadStartingMap( const std::string& mapName )
{
	m_world->InitializeAllZephyrEntityVariables();

	m_world->ChangeMap( mapName, nullptr );

	m_world->CallAllZephyrSpawnEvents();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateFromKeyboard()
{
	if ( g_devConsole->IsOpen() )
	{
		return;
	}

	if ( g_inputSystem->ConsumeAllKeyPresses( KEY_ESC ) )
	{
		g_eventSystem->FireEvent( "Quit" );
	}

	switch ( m_gameState )
	{
		case eGameState::ATTRACT:
		{
			if ( g_inputSystem->ConsumeAnyKeyJustPressed() )
			{
				ChangeGameState( eGameState::PLAYING );
			}
		}
		break;

		case eGameState::PLAYING:
		{
			if ( g_inputSystem->ConsumeAllKeyPresses( KEY_ESC ) )
			{
				ChangeGameState( eGameState::PAUSED );
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
			{
				m_isDebugRendering = !m_isDebugRendering;
			}

			if ( g_inputSystem->ConsumeAllKeyPresses( KEY_F5 ) )
			{
				ReloadGame();
				LoadStartingMap( m_startingMapName );
			}

			if ( g_inputSystem->ConsumeAllKeyPresses( KEY_F6 ) )
			{
				ReloadScripts();
			}
		}
		break;

		case eGameState::DIALOGUE:
		{
			if ( g_inputSystem->ConsumeAllKeyPresses( KEY_ESC ) )
			{
				ChangeGameState( eGameState::PLAYING );
			}

			if ( g_inputSystem->WasKeyJustPressed( KEY_F1 ) )
			{
				m_isDebugRendering = !m_isDebugRendering;
			}

			if ( g_inputSystem->ConsumeAllKeyPresses( KEY_F5 ) )
			{
				ReloadGame();
				LoadStartingMap( m_startingMapName );
			}

			if ( g_inputSystem->ConsumeAllKeyPresses( KEY_F6 ) )
			{
				ReloadScripts();
			}
		}
		break;	
	}
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMousePositions()
{
	UpdateMouseWorldPosition();
	UpdateMouseUIPosition();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMouseWorldPosition()
{
	m_mouseWorldPosition = g_inputSystem->GetNormalizedMouseClientPos() * m_worldCamera->GetOutputSize();
	m_mouseWorldPosition += m_worldCamera->GetOrthoMin();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateMouseUIPosition()
{
	m_mouseUIPosition = g_inputSystem->GetNormalizedMouseClientPos() * m_uiCamera->GetOutputSize();
}


//-----------------------------------------------------------------------------------------------
void Game::UpdateCameras()
{
	// World camera
	m_screenShakeIntensity -= SCREEN_SHAKE_ABLATION_PER_SECOND * (float)m_gameClock->GetLastDeltaSeconds();
	m_screenShakeIntensity = ClampZeroToOne( m_screenShakeIntensity );

	float maxScreenShake = m_screenShakeIntensity * MAX_CAMERA_SHAKE_DIST;
	float cameraShakeX = m_rng->RollRandomFloatInRange( -maxScreenShake, maxScreenShake );
	float cameraShakeY = m_rng->RollRandomFloatInRange( -maxScreenShake, maxScreenShake );
	Vec2 cameraShakeOffset = Vec2( cameraShakeX, cameraShakeY );

	//m_worldCamera->Translate2D( cameraShakeOffset );
	m_worldCamera->SetPosition( m_focalPoint + Vec3( cameraShakeOffset, 0.f ) );
	m_worldCamera->SetProjectionOrthographic( WINDOW_HEIGHT );
}


//-----------------------------------------------------------------------------------------------
void Game::ChangeGameState( const eGameState& newGameState )
{
	eGameState oldGameState = m_gameState;
	m_gameState = newGameState;

	switch ( newGameState )
	{
		case eGameState::LOADING:
		{
			ERROR_AND_DIE( "Tried to go back to the loading state during the game. Don't do that." );
		}
		break;

		case eGameState::ATTRACT:
		{
			// Check which state we are changing from
			switch ( oldGameState )
			{
				case eGameState::PAUSED:
				case eGameState::PLAYING:
				case eGameState::DIALOGUE:
				{
					if ( m_curMusicId != MISSING_SOUND_ID )
					{
						g_audioSystem->StopSound( m_curMusicId );
					}

					ReloadGame();
				}
				break;

				case eGameState::VICTORY:
				{
					//g_audioSystem->StopSound( m_victoryMusicID );
					
					ReloadGame();
				}
				break;
			}

			if ( m_curMusicId != MISSING_SOUND_ID )
			{
				g_audioSystem->StopSound( m_curMusicId );
			}

			SoundID attractMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/AttractMusic.mp3" );
			m_curMusicId = g_audioSystem->PlaySound( attractMusic, true, .1f );

			m_gameClock->Resume();
		}
		break;

		case eGameState::PLAYING:
		{
			m_gameClock->Resume();

			// Check which state we are changing from
			switch ( oldGameState )
			{
				case eGameState::PAUSED:
				{
					m_gameClock->Resume();

					SoundID unpause = g_audioSystem->CreateOrGetSound( "Data/Audio/Unpause.mp3" );
					g_audioSystem->PlaySound( unpause, false, .1f );
					
					if ( m_curMusicId != MISSING_SOUND_ID )
					{
						g_audioSystem->SetSoundPlaybackVolume( m_curMusicId, .1f );
					}
				}
				break;

				case eGameState::ATTRACT:
				{					
					g_devConsole->PrintString( Stringf( "Loading starting map: %s", m_startingMapName.c_str() ) );
					LoadStartingMap( m_startingMapName );

					EventArgs args;
					g_eventSystem->FireEvent( "OnGameStart", &args );
				}
				break;

				case eGameState::DIALOGUE:
				{
				}
				break;
			}
		}
		break;

		case eGameState::DIALOGUE:
		{
		}
		break;

		case eGameState::PAUSED:
		{
			if ( m_curMusicId != MISSING_SOUND_ID )
			{
				g_audioSystem->SetSoundPlaybackVolume( m_curMusicId, .05f );
			}

			SoundID pause = g_audioSystem->CreateOrGetSound( "Data/Audio/Pause.mp3" );
			g_audioSystem->PlaySound( pause, false, .1f );

			m_gameClock->Pause();
		}
		break;

		case eGameState::VICTORY:
		{
			//m_curVictoryScreenSeconds = 0.f;
			
			m_gameClock->Pause();
			/*SoundID victoryMusic = g_audioSystem->CreateOrGetSound( "Data/Audio/Victory.mp3" );
			m_victoryMusicID = g_audioSystem->PlaySound( victoryMusic, true, .1f );*/
		}
		break;
	}
}


//-----------------------------------------------------------------------------------------------
void Game::SetWorldCameraPosition( const Vec3& position )
{
	m_focalPoint = position;
}


//-----------------------------------------------------------------------------------------------
void Game::AddScreenShakeIntensity( float intensity )
{
	m_screenShakeIntensity += intensity;
}


//-----------------------------------------------------------------------------------------------
void Game::PrintToDebugInfoBox( const Rgba8& color, const std::vector< std::string >& textLines )
{
	if ( (int)textLines.size() == 0 )
	{
		return;
	}

	m_debugInfoTextBox->SetText( textLines[0], color );

	for ( int textLineIndex = 1; textLineIndex < (int)textLines.size(); ++textLineIndex )
	{
		m_debugInfoTextBox->AddLineOFText( textLines[textLineIndex], color );
	}
}


//-----------------------------------------------------------------------------------------------
void Game::WarpToMap( GameEntity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees )
{
	// No entity specified, just load the new map and set camera position and orientation
	if ( entityToWarp == nullptr )
	{
		if ( destMapName != "" )
		{
			m_world->ChangeMap( destMapName, nullptr );
		}

		return;
	}

	m_world->WarpEntityToMap( entityToWarp, destMapName, newPos, newYawDegrees );
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
float Game::GetLastDeltaSecondsf()
{
	return (float)m_gameClock->GetLastDeltaSeconds();
}


//-----------------------------------------------------------------------------------------------
GameEntity* Game::GetEntityById( EntityId id )
{
	return m_world->GetEntityById( id );
}


//-----------------------------------------------------------------------------------------------
GameEntity* Game::GetEntityByName( const std::string& name )
{
	return m_world->GetEntityByName( name );
}


//-----------------------------------------------------------------------------------------------
void Game::SaveEntityByName( GameEntity* entity )
{
	m_world->SaveEntityByName( entity );
}


//-----------------------------------------------------------------------------------------------
void Game::GetComponentFromEntityId( EventArgs* args )
{
	EntityId entityId = args->GetValue( "entityId", INVALID_ENTITY_ID );
	EntityComponentTypeId compTypeId = args->GetValue( "entityComponentTypeId", ENTITY_COMPONENT_TYPE_INVALID );

	EntityComponent* entityComponent = m_world->GetComponentFromEntityId( entityId, compTypeId );
	args->SetValue( "entityComponent", (void*)entityComponent );
}


//-----------------------------------------------------------------------------------------------
GameEntity* Game::GetEntityAtPosition( const Vec3& position ) const
{
	return m_world->GetEntityAtPosition( position );
}


//-----------------------------------------------------------------------------------------------
GameEntity* Game::GetEntityAtPosition( const Vec2& position ) const
{
	return GetEntityAtPosition( Vec3( position, 0.f ) );
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
Map* Game::GetMapByName( const std::string& name )
{
	return m_world->GetMapByName( name );
}


//-----------------------------------------------------------------------------------------------
void Game::PrintBytecodeChunk( EventArgs* args )
{
	std::string entityName = args->GetValue( "entityName", "" );
	std::string chunkName = args->GetValue( "chunkName", "" );

	if ( entityName.empty()
		 || chunkName.empty() )
	{
		return;
	}

	GameEntity* entity = GetEntityByName( entityName );
	if ( entity == nullptr )
	{
		return;
	}

	const ZephyrBytecodeChunk* chunk = ZephyrSystem::GetBytecodeChunkByName( entity->GetId(), chunkName );
	if ( chunk == nullptr )
	{
		return;
	}

	chunk->Disassemble();
}
