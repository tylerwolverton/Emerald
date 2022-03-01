#pragma once
#include "Game/GameCamera.hpp"


//-----------------------------------------------------------------------------------------------
class FreeCamera : public GameCamera
{
public:
	FreeCamera( const GameCameraSettings& gameCameraSettings );
	virtual ~FreeCamera() = default;

	virtual void Update() override;
};
