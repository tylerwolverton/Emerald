#include "Game/Core/App.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/OS/Window.hpp"
#include "Engine/Performance/PerformanceTracker.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Time/Time.hpp"
#include "Engine/Time/Clock.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrEngineEvents.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"

#include "Game/Core/GameCommon.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Scripting/ZephyrGameEvents.hpp"


//-----------------------------------------------------------------------------------------------
void App::Startup()
{
	PopulateGameConfig();
	g_physicsConfig = new PhysicsConfig();
	g_colliderFactory = new ColliderFactory();

	std::string windowTitle = g_gameConfigBlackboard.GetValue( "windowTitle", "Protogame2D" );
	float windowAspect = g_gameConfigBlackboard.GetValue( "windowAspect", 16.f / 9.f );
	float windowHeightRatio = g_gameConfigBlackboard.GetValue( "windowHeightRatio", .9f );
	eWindowMode windowMode = GetWindowModeFromGameConfig();

	Clock::MasterStartup();

	g_window = new Window();
	g_window->Open( windowTitle, windowAspect, windowHeightRatio, windowMode );

	g_eventSystem = new EventSystem();
	g_inputSystem = new InputSystem();
	g_audioSystem = new AudioSystem();
	g_renderer = new RenderContext();
	g_devConsole = new DevConsole();
	g_zephyrSubsystem = new ZephyrSubsystem();
	g_zephyrAPI = new ZephyrGameEvents();
	g_performanceTracker = new PerformanceTracker();
	g_game = new Game();

	g_eventSystem->Startup();
	g_window->SetEventSystem( g_eventSystem );

	g_inputSystem->Startup( g_window );
	g_window->SetInputSystem( g_inputSystem );

	g_audioSystem->Startup();

	g_renderer->Startup( g_window );
	DebugRenderSystemStartup( g_renderer, g_eventSystem );

	g_devConsole->Startup();
	g_devConsole->SetInputSystem( g_inputSystem );
	g_devConsole->SetRenderer( g_renderer );
	g_devConsole->SetBitmapFont( g_renderer->CreateOrGetBitmapFontFromFile( "Data/Fonts/SquirrelFixedFont" ) );
	
	// Calls g_physicsSystem2D::Startup too
	g_game->Startup();

	ZephyrSystemParams zephyrParams;
	zephyrParams.clock = g_game->GetGameClock();
	g_zephyrSubsystem->Startup( zephyrParams );

	PerformanceTrackerParams perfParams;
	perfParams.clock = g_game->GetGameClock();
	g_performanceTracker->StartUp( perfParams );

	g_eventSystem->RegisterEvent( "Quit", "Quit the game.", eUsageLocation::EVERYWHERE, QuitGame );
}


//-----------------------------------------------------------------------------------------------
void App::Shutdown()
{
	g_zephyrSubsystem->Shutdown();
	g_game->Shutdown();
	g_devConsole->Shutdown();
	DebugRenderSystemShutdown();
	g_renderer->Shutdown();
	g_audioSystem->Shutdown();
	g_inputSystem->Shutdown();
	g_eventSystem->Shutdown();
	g_window->Close();

	Clock::MasterShutdown();
		
	PTR_SAFE_DELETE( g_game );
	PTR_SAFE_DELETE( g_colliderFactory );
	PTR_SAFE_DELETE( g_physicsConfig );
	PTR_SAFE_DELETE( g_performanceTracker );
	PTR_SAFE_DELETE( g_zephyrAPI );
	PTR_SAFE_DELETE( g_zephyrSubsystem );
	PTR_SAFE_DELETE( g_devConsole );
	PTR_SAFE_DELETE( g_renderer );
	PTR_SAFE_DELETE( g_audioSystem );
	PTR_SAFE_DELETE( g_inputSystem );
	PTR_SAFE_DELETE( g_eventSystem );
	PTR_SAFE_DELETE( g_window );
}


//-----------------------------------------------------------------------------------------------
void App::RunFrame()
{
	BeginFrame();											// for all engine systems (NOT the game)
	Update();												// for the game only
	Render();												// for the game only
	EndFrame();												// for all engine systems (NOT the game)
}


//-----------------------------------------------------------------------------------------------
bool App::HandleQuitRequested()
{
	m_isQuitting = true;

	return 0;
}


//-----------------------------------------------------------------------------------------------
void App::RestartGame()
{
	g_game->Shutdown();
	delete g_game;

	g_game = new Game();
	g_game->Startup();
}


//-----------------------------------------------------------------------------------------------
void App::BeginFrame()
{
	Clock::MasterBeginFrame();

	g_window->BeginFrame();
	g_eventSystem->BeginFrame();
	g_devConsole->BeginFrame();
	g_inputSystem->BeginFrame();
	g_audioSystem->BeginFrame();
	g_renderer->BeginFrame();
	DebugRenderBeginFrame();
}


//-----------------------------------------------------------------------------------------------
void App::Update()
{
	g_devConsole->Update();
	g_game->Update();
	g_zephyrSubsystem->Update();
	g_performanceTracker->Update();

	UpdateFromKeyboard();
}


//-----------------------------------------------------------------------------------------------
void App::UpdateFromKeyboard()
{
	if ( g_inputSystem->WasKeyJustPressed( KEY_ESC ) )
	{
		if ( g_devConsole->IsOpen() )
		{
			g_devConsole->Close();
			g_inputSystem->ConsumeAnyKeyJustPressed();
		}
	}
	
	if ( g_inputSystem->WasKeyJustPressed( KEY_TILDE ) )
	{
		g_devConsole->ToggleOpenFull();
		g_inputSystem->ConsumeAnyKeyJustPressed();
	}
	
	if ( g_inputSystem->WasKeyJustPressed( KEY_F11 ) )
	{
		g_window->ToggleWindowMode();
	}
}


//-----------------------------------------------------------------------------------------------
void App::Render() const
{
	g_game->Render();
	g_performanceTracker->Render();
	DebugRenderScreenTo( g_renderer->GetBackBuffer() );
	g_devConsole->Render();
}


//-----------------------------------------------------------------------------------------------
void App::EndFrame()
{
	DebugRenderEndFrame();
	g_renderer->EndFrame();
	g_audioSystem->EndFrame();
	g_inputSystem->EndFrame();
	g_devConsole->EndFrame();
	g_eventSystem->EndFrame();
	g_window->EndFrame();
}


//-----------------------------------------------------------------------------------------------
eWindowMode App::GetWindowModeFromGameConfig()
{
	std::string windowModeStr = g_gameConfigBlackboard.GetValue( "windowMode", "windowed" );

	if ( !_strcmpi( windowModeStr.c_str(), "windowed" ) )
	{
		return eWindowMode::WINDOWED;
	}
	else if ( !_strcmpi( windowModeStr.c_str(), "borderless" ) )
	{
		return eWindowMode::BORDERLESS;
	}
	else
	{
		g_devConsole->PrintWarning( Stringf( "Unrecognized window mode '%s' found in game config; using windowed mode.", windowModeStr.c_str() ) );
		return eWindowMode::WINDOWED;
	}
}


//-----------------------------------------------------------------------------------------------
bool App::QuitGame( EventArgs* args )
{
	UNUSED( args );
	g_app->HandleQuitRequested();

	return 0;
}
