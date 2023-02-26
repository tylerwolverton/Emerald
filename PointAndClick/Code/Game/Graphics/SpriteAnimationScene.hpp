#pragma once
#include <vector>


//-----------------------------------------------------------------------------------------------
struct SpriteAnimationComponentDefinition;
struct SpriteAnimationComponent;
class Entity;


//-----------------------------------------------------------------------------------------------
struct SpriteAnimationScene
{
public:
	std::vector<SpriteAnimationComponent*> animComponents;

public:
	SpriteAnimationComponent* CreateAndAddComponent( Entity* parentEntity, const SpriteAnimationComponentDefinition& componentDef );
	void AddComponent( SpriteAnimationComponent* spriteAnimComp );
};
