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
void FPSCamera::UpdateTranslation( Entity* target )
{
	SetPosition( target->GetPosition() + Vec3( 0.f, 0.f, target->GetEyeHeight() ) );
}
