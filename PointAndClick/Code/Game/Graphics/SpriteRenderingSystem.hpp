#pragma once
#include <vector>


//-----------------------------------------------------------------------------------------------
struct SpriteAnimationScene;
class GameEntity;


//-----------------------------------------------------------------------------------------------
class SpriteRenderingSystem
{
public:
	static void RenderScene( const SpriteAnimationScene& spriteAnimScene, const std::vector<GameEntity*>& sceneEntities );
};
