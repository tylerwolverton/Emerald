#pragma once
#include "Engine/Framework/EntityComponent.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
struct SpriteAnimationComponentDefinition;
class SpriteAnimationSetDefinition;


//-----------------------------------------------------------------------------------------------
struct SpriteAnimationComponent : public EntityComponent
{
public:
	const SpriteAnimationComponentDefinition&	spriteAnimCompDef;
	SpriteAnimationSetDefinition*				curSpriteAnimSetDef = nullptr;
	float										cumulativeAnimSeconds = 0.f;

public:
	SpriteAnimationComponent( const EntityId& parentEntityId, const SpriteAnimationComponentDefinition& spriteAnimCompDef );

	void ChangeSpriteAnimation( const std::string& newSpriteAnimDefSetName );
	void PlaySpriteAnimation(	const std::string& newSpriteAnimDefSetName );
};
