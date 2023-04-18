#include "Game/Graphics/SpriteAnimationSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Framework/Entity.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"

#include "Game/Core/GameCommon.hpp"
#include "Game/Graphics/SpriteAnimationComponent.hpp"
#include "Game/Graphics/SpriteAnimationScene.hpp"
#include "Game/Graphics/SpriteAnimationSetDefinition.hpp"


//-----------------------------------------------------------------------------------------------
void SpriteAnimationSystem::AdvanceAnimations( SpriteAnimationScene& spriteAnimScene, float deltaSeconds )
{
	for ( SpriteAnimationComponent* spriteAnimComp : spriteAnimScene.animComponents )
	{
		spriteAnimComp->cumulativeAnimSeconds += deltaSeconds;
		if ( spriteAnimComp->curSpriteAnimSetDef != nullptr )
		{
			SpriteAnimDefinition* animDef = spriteAnimComp->curSpriteAnimSetDef->GetSpriteAnimationDefForDirection( Vec2::ZERO );
			int frameIndex = animDef->GetFrameIndexAtTime( spriteAnimComp->cumulativeAnimSeconds );

			spriteAnimComp->curSpriteAnimSetDef->FireFrameEvent( frameIndex, spriteAnimComp->GetParentEntityId() );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimationSystem::ChangeSpriteAnimation( const EntityId& entityId, const std::string& spriteAnimDefSetName )
{
	SpriteAnimationComponent* spriteAnimComp = (SpriteAnimationComponent*)GetComponentFromEntityId( entityId, ENTITY_COMPONENT_TYPE_SPRITE_ANIM );

	if ( spriteAnimComp == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Entity '%s' has no sprite animation component to change", Entity::GetName( entityId ).c_str() ) );
		return;
	}

	spriteAnimComp->ChangeSpriteAnimation( spriteAnimDefSetName );
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimationSystem::PlaySpriteAnimation( const EntityId& entityId, const std::string& spriteAnimDefSetName )
{
	SpriteAnimationComponent* spriteAnimComp = (SpriteAnimationComponent*)GetComponentFromEntityId( entityId, ENTITY_COMPONENT_TYPE_SPRITE_ANIM );

	if ( spriteAnimComp == nullptr )
	{
		g_devConsole->PrintError( Stringf( "Entity '%s' has no sprite animation component to play", Entity::GetName( entityId ).c_str() ) );
		return;
	}
	
	spriteAnimComp->PlaySpriteAnimation( spriteAnimDefSetName );
}
