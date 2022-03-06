#pragma once
#include "Game/GameCamera.hpp"


//-----------------------------------------------------------------------------------------------
class Entity;

//-----------------------------------------------------------------------------------------------
class FPSCamera : public GameCamera
{
public:
	FPSCamera( const GameCameraSettings& gameCameraSettings );
	virtual ~FPSCamera() = default;

	virtual void Update( Entity* target = nullptr ) override;
};
