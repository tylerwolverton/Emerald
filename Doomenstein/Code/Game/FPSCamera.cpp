#include "Game/FPSCamera.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Time/Clock.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"


//-----------------------------------------------------------------------------------------------
FPSCamera::FPSCamera( const GameCameraSettings& gameCameraSettings )
	: GameCamera( gameCameraSettings )
{
}


//-----------------------------------------------------------------------------------------------
void FPSCamera::Update( Entity* target )
{
	// Rotation
	Vec2 mousePosition = g_inputSystem->GetMouseDeltaPosition();
	float pitchDegrees = mousePosition.y; // *s_mouseSensitivityMultiplier;
	pitchDegrees *= .009f;

	SetPosition( target->GetPosition() + Vec3( 0.f, 0.f, target->GetEyeHeight() ) );
	SetYawOrientationDegrees( target->GetOrientationDegrees() );
	RotateYawPitchRoll( 0.f, pitchDegrees, 0.f );
}
