#include "Game/Graphics/SpriteAnimationScene.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Framework/Entity.hpp"

#include "Game/Graphics/SpriteAnimationComponent.hpp"


//-----------------------------------------------------------------------------------------------
SpriteAnimationComponent* SpriteAnimationScene::CreateAndAddComponent( Entity* parentEntity, const SpriteAnimationComponentDefinition& componentDef )
{
	if ( parentEntity == nullptr )
	{
		g_devConsole->PrintError( "Can't create a sprite animation component with a null parent entity" );
		return nullptr;
	}

	SpriteAnimationComponent* spriteAnimComp = new SpriteAnimationComponent( parentEntity->GetId(), componentDef );
	if ( spriteAnimComp != nullptr )
	{
		animComponents.push_back( spriteAnimComp );
	}
	return spriteAnimComp;
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimationScene::Destroy()
{
	PTR_VECTOR_SAFE_DELETE( animComponents );
}