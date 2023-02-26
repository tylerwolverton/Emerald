#include "Game/Graphics/SpriteRenderingSystem.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Renderer/SpriteDefinition.hpp"

#include "Game/Core/GameCommon.hpp"
#include "Game/Framework/GameEntity.hpp"
#include "Game/Graphics/SpriteAnimationComponent.hpp"
#include "Game/Graphics/SpriteAnimationComponentDefinition.hpp"
#include "Game/Graphics/SpriteAnimationScene.hpp"
#include "Game/Graphics/SpriteAnimationSetDefinition.hpp"


//-----------------------------------------------------------------------------------------------
void SpriteRenderingSystem::RenderScene( const SpriteAnimationScene& spriteAnimScene, const std::vector<GameEntity*>& sceneEntities )
{
	for ( SpriteAnimationComponent* spriteAnimComp : spriteAnimScene.animComponents )
	{
		if ( spriteAnimComp->curSpriteAnimSetDef == nullptr )
		{
			return;
		}

		SpriteAnimDefinition* animDef = spriteAnimComp->curSpriteAnimSetDef->GetSpriteAnimationDefForDirection( Vec2::ZERO );
		
		const SpriteDefinition& spriteDef = animDef->GetSpriteDefAtTime( spriteAnimComp->cumulativeAnimSeconds );

		Vec2 mins, maxs;
		spriteDef.GetUVs( mins, maxs );

		std::vector<Vertex_PCU> vertexes;
		AppendVertsForAABB2D( vertexes, spriteAnimComp->spriteAnimCompDef.localDrawBounds, Rgba8::WHITE, mins, maxs );

		// HACK: Find a better way to get transforms into this function
		Vec3 position = Vec3::ZERO;
		for ( GameEntity* entity : sceneEntities )
		{
			if ( spriteAnimComp->GetParentEntityId() == entity->GetId() )
			{
				position = entity->GetPosition();
				break;
			}
		}

		Vertex_PCU::TransformVertexArray( vertexes, 1.f, 0.f, position );

		g_renderer->BindTexture( 0, &( spriteDef.GetTexture() ) );
		g_renderer->DrawVertexArray( vertexes );
	}
}
