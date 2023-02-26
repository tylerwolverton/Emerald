#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
struct SpriteAnimationScene;


//-----------------------------------------------------------------------------------------------
class SpriteAnimationSystem
{
public:
	static void AdvanceAnimations( SpriteAnimationScene& spriteAnimScene, float deltaSeconds );

	static void ChangeSpriteAnimation( const EntityId& entityId, const std::string& spriteAnimDefSetName );
	static void PlaySpriteAnimation( const EntityId& entityId, const std::string& spriteAnimDefSetName );
	//SpriteAnimationSetDefinition* GetSpriteAnimSetDef( const std::string& animSetName ) const { return m_entityDef.GetSpriteAnimSetDef( animSetName ); }
};
