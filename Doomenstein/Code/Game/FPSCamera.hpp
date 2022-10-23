#pragma once
#include "Game/GameCamera.hpp"


//-----------------------------------------------------------------------------------------------
class GameEntity;

//-----------------------------------------------------------------------------------------------
class FPSCamera : public GameCamera
{
public:
	FPSCamera( const GameCameraSettings& gameCameraSettings );
	virtual ~FPSCamera() = default;

	virtual void UpdateTranslation( GameEntity* target = nullptr ) override;
};
