#include "Game/EntityController.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Input/InputSystem.hpp"

#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/GameEntity.hpp"


//-----------------------------------------------------------------------------------------------
void EntityController::Update()
{
	if ( m_controlledEntity == nullptr )
	{
		return;
	}
	
	if ( !g_devConsole->IsOpen() )
	{
		if ( g_inputSystem->ConsumeKeyPress( MOUSE_LBUTTON ) )
		{
			EventArgs args;
			args.SetValue( "newTargetLocation", g_game->GetMouseWorldPosition() );

			m_controlledEntity->FireScriptEvent( "SetTargetLocation", &args );
		}
	}
}


