#pragma once
#include "Game/GameCamera.hpp"


//-----------------------------------------------------------------------------------------------
class Entity;


//-----------------------------------------------------------------------------------------------
class FreeCamera : public GameCamera
{
public:
	FreeCamera( const GameCameraSettings& gameCameraSettings );
	virtual ~FreeCamera() = default;

	virtual void Update( Entity* target = nullptr ) override;

private:
	void UpdateMovementFromKeyboard();
};
