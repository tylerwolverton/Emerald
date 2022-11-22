#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrEngineEvents.hpp"

#include <unordered_set>
#include <string>


//-----------------------------------------------------------------------------------------------
class Entity;
class GameEntity;


//-----------------------------------------------------------------------------------------------
class ZephyrGameEvents : public ZephyrEngineEvents
{
public:
	ZephyrGameEvents();
	virtual ~ZephyrGameEvents();
	
	virtual Entity* GetEntityById( const EntityId& id ) const override;
	virtual Entity* GetEntityByName( const std::string& name ) const override;

private:
	// Zephyr Script Events
	//void ChangeZephyrScriptState( EventArgs* args ); // Should be called directly in VM as opposed to an event

	// Debug Events
	void PrintDebugText( EventArgs* args );
	void PrintDebugScreenText( EventArgs* args );
	void PrintToConsole( EventArgs* args );

	// Math
	void GetVec2Normalized( EventArgs* args );

	// Game Events
	//void SpawnEntity( EventArgs* args );
	void StartDialogue( EventArgs* args );
	void EndDialogue( EventArgs* args );
	void StartNewTimer( EventArgs* args );
	void WinGame( EventArgs* args );

	// Entity Events
	void GetNativeEntityVariable( EventArgs* args );
	void SetNativeEntityVariable( EventArgs* args );
	void GetEntityUnderMouse( EventArgs* args );
	void DestroySelf( EventArgs* args );

	// AI
	void MoveToLocation( EventArgs* args );
	void PathToLocation( EventArgs* args );
	void MoveInDirection( EventArgs* args );
	//void GetEntityLocation( EventArgs* args );

	// Input
	void RegisterKeyEvent( EventArgs* args );
	void UnRegisterKeyEvent( EventArgs* args );
	void GetMouseCursorPositionWorld( EventArgs* args );
	void GetMouseCursorPositionUI( EventArgs* args );

	// Audio/Visual
	void ChangeSpriteAnimation( EventArgs* args );
	void PlaySpriteAnimation( EventArgs* args );
	void AddAnimationEvent( EventArgs* args );
	void PlaySound( EventArgs* args );
	void ChangeMusic( EventArgs* args );
	void AddScreenShake( EventArgs* args );

private:
	GameEntity* GetTargetEntityFromArgs( EventArgs* args );
};
