#pragma once
#include "Game/GameCamera.hpp"


//-----------------------------------------------------------------------------------------------
struct Vec3;
class Entity;


//-----------------------------------------------------------------------------------------------
class FreeCamera : public GameCamera
{
public:
	FreeCamera( const GameCameraSettings& gameCameraSettings );
	virtual ~FreeCamera() = default;

	virtual void UpdateTranslation( Entity* target = nullptr ) override;

private:
	Vec3 GetMovementFromKeyboard();
};
