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
	~SpriteAnimationScene();

	SpriteAnimationComponent*	CreateAndAddComponent( Entity* parentEntity, const SpriteAnimationComponentDefinition& componentDef );
	void						Destroy();
};
