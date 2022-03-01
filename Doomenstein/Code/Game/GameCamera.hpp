#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/Camera.hpp"


//-----------------------------------------------------------------------------------------------
enum eGameCameraType
{
	FREE,
	FPS
};


//-----------------------------------------------------------------------------------------------
struct GameCameraSettings
{
	Vec2 outputSize = Vec2( 16.f, 9.f );
	float fovDegrees = 90.f;
	float nearClipZ = -.05f;
	float farClipZ = -100.f;
	eGameCameraType gameCameraType = eGameCameraType::FREE;
	eCameraType cameraType = eCameraType::WORLD;
};


//-----------------------------------------------------------------------------------------------
class GameCamera
{
	friend class Game;

public:
	GameCamera( const GameCameraSettings& gameCameraSettings );
	virtual ~GameCamera();

	void BeginCamera();
	void EndCamera();
	void DebugRenderWorld();
	virtual void Update() = 0;

	// Wrappers for engine camera
	const Transform GetTransform() const																{ return m_engineCamera->GetTransform(); }

	void SetPosition( const Vec3& position )															{ m_engineCamera->SetPosition( position ); }
	void Translate( const Vec3& translation )															{ m_engineCamera->Translate( translation ); }
	void Translate2D( const Vec2& translation )															{ m_engineCamera->Translate2D( translation ); }
	void SetPitchRollYawOrientationDegrees( float pitchDegrees, float rollDegrees, float yawDegrees )	{ m_engineCamera->SetPitchRollYawOrientationDegrees( pitchDegrees, rollDegrees, yawDegrees ); }
	void SetYawPitchRollOrientationDegrees( float yawDegrees, float pitchDegrees, float rollDegrees )	{ m_engineCamera->SetYawPitchRollOrientationDegrees( yawDegrees, pitchDegrees, rollDegrees ); }
	void SetYawOrientationDegrees( float yawDegrees )													{ m_engineCamera->SetYawOrientationDegrees( yawDegrees ); }
	void RotateYawPitchRoll( float yawDegrees, float pitchDegrees, float rollDegrees )					{ m_engineCamera->RotateYawPitchRoll( yawDegrees, pitchDegrees, rollDegrees ); }
	void SetTransform( const Transform& transform )														{ m_engineCamera->SetTransform( transform ); }

	void SetColorTarget( Texture* texture )																{ m_engineCamera->SetColorTarget( texture ); }
	void SetColorTarget( int index, Texture* texture )													{ m_engineCamera->SetColorTarget( index, texture ); }

private:
	// Temp: Remove access to this from Game
	Camera* GetEngineCamera() const						{ return m_engineCamera; }

private:
	Camera* m_engineCamera = nullptr;
};
