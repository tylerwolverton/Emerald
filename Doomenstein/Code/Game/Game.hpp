#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Time/Timer.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Game/GameCommon.hpp"
#include "Game/GameCamera.hpp"
#include "Game/EntityController.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;
class Clock;
class Entity;
class RandomNumberGenerator;
class Camera;
class GPUMesh;
class Material;
class TextBox;
class Texture;
class UIPanel;
class UISystem;
class Map;
class World;


//-----------------------------------------------------------------------------------------------
enum class eGameState
{
	LOADING,
	ATTRACT,
	PLAYING,
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

	void			Startup();
	void			Update();
	void			Render() const;
	void			DebugRender() const;
	void			Shutdown();

	void			RestartGame();
	
	const Vec2		GetMouseWorldPosition()														{ return m_mouseWorldPosition; }
	const Camera*	GetWorldCamera()															{ return m_playerController->GetCurrentWorldCamera()->GetEngineCamera(); }
	Clock*			GetGameClock()																{ return m_gameClock; }

	void			AddScreenShakeIntensity( float additionalIntensityFraction );
	
	void			SetCameraPositionAndYaw( const Vec2& pos, float yaw );
	void			SetCameraPositionAndYaw( const Vec3& pos, float yaw );
	
	void			WarpToMap		( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees );
	void			WarpToMap		( Entity* entityToWarp, const std::string& destMapName, const Vec3& newPos, float newYawDegrees );
	void			WarpEntityToMap	( Entity* entityToWarp, const std::string& destMapName, const Vec2& newPos, float newYawDegrees );
	void			WarpEntityToMap	( Entity* entityToWarp, const std::string& destMapName, const Vec3& newPos, float newYawDegrees );

	Entity*			GetEntityById( EntityId id );
	Entity*			GetEntityByName( const std::string& name );
	Map*			GetMapByName( const std::string& name );
	Map*			GetCurrentMap();
	void			SaveEntityByName( Entity* entity );
	void			PushCamera( const GameCameraSettings& gameCameraSettings );
	void			PopCamera();

	void			PlaySoundByName	( const std::string& soundName, bool isLooped = false, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );
	void			ChangeMusic		( const std::string& musicName, bool isLooped = true, float volume = 1.f, float balance = 0.0f, float speed = 1.0f, bool isPaused = false );

	int				AcquireAndSetLightFromPool( const Light& newLight );
	void			FreeLight( int lightIdx );
	void			FreeAllLights();

	static bool		SetMouseSensitivity( EventArgs* args );
	static bool		SetAmbientLightColor( EventArgs* args );
	
public:
	RandomNumberGenerator* m_rng = nullptr;
	bool g_raytraceFollowCamera = true;

	static float m_mouseSensitivityMultiplier;

private:
	void LoadAssets();
	void LoadSounds();
	void LoadXmlUIElements();
	void LoadXmlEntityTypes();
	void LoadXmlMapMaterials();
	void LoadXmlMapRegions();
	void LoadXmlMaps();
	void LoadWorldDefinitionFromXml();
	void LoadAndCompileZephyrScripts();
	void ReloadGame();
	void ReloadScripts();

	void ChangeMap( const std::string& mapName );

	void InitializeCameras();

	void AddGunToUI();

	void UpdateFromKeyboard();
	void LoadStartingMap( const std::string& mapName );

	void RenderDebugUI() const;

	void UpdateCameras();
	void TranslateCameraFPS( const Vec3& relativeTranslation );
	void SetLightDirectionToCamera( Light& light );
		
	// Events
	void WarpMapCommand( EventArgs* args );

private:
	Clock* m_gameClock = nullptr;

	EntityController* m_playerController = nullptr;

	eGameState m_gameState = eGameState::LOADING;
	bool m_isDebugRendering = false;

	// UI
	UISystem* m_uiSystem = nullptr;
	UIPanel* m_hudUIPanel = nullptr;
	UIPanel* m_worldUIPanel = nullptr;
	
	Vec2 m_mouseWorldPosition = Vec2::ZERO;
	Vec2 m_mouseUIPosition = Vec2::ZERO;

	float m_screenShakeIntensity = 0.f;

	Camera* m_uiCamera = nullptr;

	World* m_world = nullptr;
	std::string m_startingMapName;
			
	Rgba8 m_ambientColor = Rgba8::WHITE;
	float m_ambientIntensity = 0.01f;
	Light m_lightPool[MAX_LIGHTS];
	float m_gamma = 2.2f;

	// Default map data
	std::string m_defaultMaterialStr;
	std::string m_defaultMapRegionStr;
	std::string m_defaultMapRegionCollisionLayerStr;

	// Audio
	std::string m_curMusicName;
	SoundPlaybackID m_curMusicId = (SoundPlaybackID)-1;

	std::map<std::string, SoundID> m_loadedSoundIds;
};
