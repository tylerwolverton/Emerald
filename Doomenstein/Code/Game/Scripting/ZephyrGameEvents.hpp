#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrEngineEvents.hpp"

#include <unordered_set>
#include <string>


//-----------------------------------------------------------------------------------------------
class Entity;
class GameEntity;


//-----------------------------------------------------------------------------------------------
// REFACTOR: Can we reduce this to static lambda registration?
class ZephyrGameEvents : public ZephyrEngineEvents
{
public:
	ZephyrGameEvents();
	virtual ~ZephyrGameEvents();
	
	virtual Entity* GetEntityById( const EntityId& id ) const override;
	virtual Entity* GetEntityByName( const std::string& name ) const override;

private:
	// Zephyr Script Events
	void ChangeZephyrScriptState( EventArgs* args );

	// Debug Events
	void PrintDebugText( EventArgs* args );
	void PrintDebugScreenText( EventArgs* args );
	void PrintToConsole( EventArgs* args );

	// Game Events
	//void SpawnEntity( EventArgs* args );
	/*void StartDialogue( EventArgs* args );
	void EndDialogue( EventArgs* args );
	void AddLineOfDialogueText( EventArgs* args );
	void AddDialogueChoice( EventArgs* args );*/
	void StartNewTimer( EventArgs* args );
	void WinGame( EventArgs* args );

	// Entity Events
	void DestroySelf( EventArgs* args );
	void Die( EventArgs* args );
	//void DamageEntity( EventArgs* args );
	void WarpToMap( EventArgs* args );
	void RotateEntity( EventArgs* args );
	void MoveInCircle( EventArgs* args );
	void MoveInDirection( EventArgs* args );
	void MoveInRelativeDirection( EventArgs* args );
	void AddImpulse( EventArgs* args );
	void GetEntityFromCameraRaytrace( EventArgs* args );
	void GetEntityFromRaytrace( EventArgs* args );

	// Input
	void RegisterKeyEvent( EventArgs* args );
	void UnRegisterKeyEvent( EventArgs* args );

	// Audio/Visual
	void ChangeSpriteAnimation( EventArgs* args );
	void PlaySpriteAnimation( EventArgs* args );
	void AddAnimationEvent( EventArgs* args );
	void PlaySound( EventArgs* args );
	void ChangeMusic( EventArgs* args );
	void AddScreenShake( EventArgs* args );
	void ChangeLight( EventArgs* args );

	// Camera
	void PushCamera( EventArgs* args );
	void PopCamera( EventArgs* args );

private:
	GameEntity* GetTargetEntityFromArgs( EventArgs* args );
};