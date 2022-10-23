#include "Game/FreeCamera.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Time/Clock.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

//-----------------------------------------------------------------------------------------------
FreeCamera::FreeCamera( const GameCameraSettings& gameCameraSettings )
	: GameCamera( gameCameraSettings )
{
}


//-----------------------------------------------------------------------------------------------
void FreeCamera::UpdateTranslation( GameEntity* target )
{
	UNUSED( target );

	Vec3 movementTranslation = GetMovementFromKeyboard();

	// Translation
	Vec3 forwardVector = GetTransform().GetForwardVector();
	Vec3 rightVector = GetTransform().GetRightVector();
	Vec3 upVector = GetTransform().GetUpVector();

	Vec3 finalMovementVector( forwardVector * movementTranslation.x
							  + rightVector * movementTranslation.y
							  + upVector * movementTranslation.z );

	float deltaSeconds = (float)g_game->GetGameClock()->GetLastDeltaSeconds();
	
	Translate( finalMovementVector * deltaSeconds );
}


//-----------------------------------------------------------------------------------------------
Vec3 FreeCamera::GetMovementFromKeyboard()
{
	Vec3 movementTranslation;
	if ( g_inputSystem->IsKeyPressed( 'D' ) )
	{
		movementTranslation.y += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'A' ) )
	{
		movementTranslation.y -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'W' ) )
	{
		movementTranslation.x += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'S' ) )
	{
		movementTranslation.x -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( KEY_SPACEBAR ) )
	{
		movementTranslation.z += 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( 'C' ) )
	{
		movementTranslation.z -= 1.f;
	}

	if ( g_inputSystem->IsKeyPressed( KEY_SHIFT ) )
	{
		movementTranslation *= 10.f;
	}

	return movementTranslation;
}
