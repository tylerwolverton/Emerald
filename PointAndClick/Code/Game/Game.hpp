#pragma once
#include "Engine/Audio/AudioCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/UI/UISystem.hpp"

#include "Game/GameCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
struct Rgba8;
class GameEntity;
class Map;
class RandomNumberGenerator;
class Clock;
class Camera;
class TextBox;
class UISystem;
class World;


//-----------------------------------------------------------------------------------------------
enum class eGameState
{
	LOADING,
	ATTRACT,
	PLAYING,
	DIALOGUE,
	GAME_OVER,
	VICTORY,
	PAUSED
};


//-----------------------------------------------------------------------------------------------
class Game
{
public:
	Game();
	~Game();

	void		Startup();
	void		BeginFrame();
	void		Update();
	void		Render() const;
	void		DebugRender() const;
	void		EndFrame();
	void		Shutdown();

	void		RestartGame();
	
	void		ChangeGameState( const eGameState& newGameState );
	eGameState  GetGameState()															{ return m_gameState; }

	const Vec2	GetMouseWorldPosition()													{ return m_mouseWorldPosition; }
	const Vec2	GetMouseUIPosition()													{ return m_mouseUIPosition; }
	Clock*		GetGameClock()															{ return m_gameClock; }

	void		SetWorldCameraPosition( const Vec3& position );
	void		AddScreenShakeIntensity( float additionalIntensityFraction );

	void		PrintToDebugInfoBox( const Rgba8& color, const std::vector< std::string >& textLines );

	void		WarpToMap( GameEntity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees );

	float		GetLastDeltaSecondsf();

	GameEntity*	GetEntityById( EntityId id );
	GameEntity*	GetEntityByName( const std::string& name );
	Map*		GetMapByName( const std::string& name );
	Map*		GetCurrentMap();
	void		SaveEntityByName( GameEntity* entity );

	void		PlaySoundByName( const std::string& soundName, bool isLooped = false, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );
	void		ChangeMusic( const std::string& musicName, bool isLooped = true, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );

	void		GetComponentFromEntityId( EventArgs* args );

public:
	RandomNumberGenerator* m_rng = nullptr;

private:
	void InitializeCameras();

	void LoadAssets();
	void LoadSounds();
	void LoadMapsFromXml();
	void LoadEntityTypesFromXml();
	void LoadWorldDefinitionFromXml();
	void LoadAndCompileZephyrScripts();
	void ReloadGame();
	void ReloadScripts();

	void UpdateFromKeyboard();
	void LoadStartingMap( const std::string& mapName );
	void UpdateMousePositions();
	void UpdateMouseWorldPosition();
	void UpdateMouseUIPosition();
	void UpdateCameras();

	void InitializeUI();

	// Events
	void PrintBytecodeChunk( EventArgs* args );

private:
	Clock* m_gameClock = nullptr;

	bool m_isPaused = false;
	bool m_isDebugRendering = false;

	// HUD
	UISystem* m_uiSystem = nullptr;
	UIPanel* m_uiInfoPanel = nullptr;

	TextBox* m_debugInfoTextBox = nullptr;

	Vec2 m_mouseWorldPosition = Vec2::ZERO;
	Vec2 m_mouseUIPosition = Vec2::ZERO;

	float m_screenShakeIntensity = 0.f;

	int m_loadingFrameNum = 0;

	Camera* m_worldCamera = nullptr;
	Camera* m_uiCamera = nullptr;
	Vec3 m_focalPoint = Vec3::ZERO;

	// Default map data
	std::string m_defaultTileName;
	std::string m_defaultTileMaterialName;

	// Audio
	std::string m_curMusicName;
	SoundPlaybackID m_curMusicId = (SoundPlaybackID)-1;

	std::map<std::string, SoundID> m_loadedSoundIds;

	eGameState m_gameState = eGameState::LOADING;

	GameEntity* m_player = nullptr;

	World* m_world = nullptr;
	std::string m_startingMapName;
};
