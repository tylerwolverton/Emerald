#include "Game/LineMap.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Time/Clock.hpp"

#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game/MapData.hpp"
#include "Game/MapMaterialTypeDefinition.hpp"
#include "Game/World.hpp"


//-----------------------------------------------------------------------------------------------
LineMap::LineMap( const MapData& mapData, World* world )
	: Map( mapData, world )
{
	m_walls = mapData.walls;
	m_regionTypeDefs = mapData.regionTypeDefs;
}


//-----------------------------------------------------------------------------------------------
LineMap::~LineMap()
{

}


//-----------------------------------------------------------------------------------------------
void LineMap::Load()
{
	g_game->SetCameraPositionAndYaw( m_playerStartPos, m_playerStartYaw );
}


//-----------------------------------------------------------------------------------------------
void LineMap::Unload()
{

}


//-----------------------------------------------------------------------------------------------
void LineMap::Update( float deltaSeconds )
{
	Map::Update( deltaSeconds );

	ApplyGravityToEntities();
	ResolveEntityVsWallCollisions();

	if ( g_game->g_raytraceFollowCamera )
	{
		m_raytraceTransform = g_game->GetWorldCamera()->GetTransform();
	}
}


//-----------------------------------------------------------------------------------------------
void LineMap::UpdateMeshes()
{
	if ( !m_isMeshDirty )
	{
		return;
	}

	m_mesh.clear();

	for ( int wallIdx = 0; wallIdx < (int)m_walls.size(); ++wallIdx )
	{
		Vec2 uvsAtMins, uvsAtMaxs;
		MapMaterialTypeDefinition* materialTypeDef = m_regionTypeDefs[wallIdx]->GetSideMaterial();
		if ( materialTypeDef == nullptr )
		{
			return;
		}

		materialTypeDef->GetSpriteSheet()->GetSpriteUVs( uvsAtMins, uvsAtMaxs, materialTypeDef->GetSpriteCoords() );

		AppendVertsForOBB3D( m_mesh, m_walls[wallIdx], Rgba8::WHITE, uvsAtMins, uvsAtMaxs );
	}

	m_isMeshDirty = false;
}


//-----------------------------------------------------------------------------------------------
void LineMap::Render() const
{
	Map::Render();

	if ( m_mesh.size() == 0 )
	{
		return;
	}

	g_renderer->SetModelMatrix( Mat44::IDENTITY );
	g_renderer->SetBlendMode( eBlendMode::ALPHA );
	g_renderer->BindShaderProgram( g_renderer->GetOrCreateShaderProgram( "Data/Shaders/src/Lit.hlsl" ) );

	g_renderer->BindTexture( 0, g_renderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8.png" ) );
	g_renderer->BindTexture( 1, g_renderer->CreateOrGetTextureFromFile( "Data/Images/Terrain_8x8_n.png" ) );

	g_renderer->DrawVertexArray( m_mesh );
}


//-----------------------------------------------------------------------------------------------
// TODO: Make this a 2D top down view to check map?
void LineMap::DebugRender() const
{
	Map::DebugRender();

	RaycastResult result = Raycast( m_raytraceTransform.GetPosition(), m_raytraceTransform.GetForwardVector(), 5.f );
	if ( result.didImpact )
	{
		DebugAddWorldPoint( result.impactPos, Rgba8::PURPLE );
		DebugAddWorldArrow( result.impactPos, result.impactPos + result.impactSurfaceNormal * .5f, Rgba8::ORANGE );

		if ( !g_game->g_raytraceFollowCamera )
		{
			DebugAddWorldArrow( m_raytraceTransform.GetPosition(), result.impactPos, Rgba8::MAGENTA );
		}
	}

	for ( int wallIdx = 0; wallIdx < (int)m_walls.size(); ++wallIdx )
	{
		Vec3 points[8];
		m_walls[wallIdx].GetCornerPositions( points );
		for ( int pointIdx = 0; pointIdx < 8; ++pointIdx )
		{
			DebugAddWorldPoint( points[pointIdx], Rgba8::GREEN );
		}

		DebugAddWorldWireSphere( m_walls[wallIdx].GetCenter(), m_walls[wallIdx].GetOuterRadius(), Rgba8::GREEN );
	}
}


//-----------------------------------------------------------------------------------------------
RaycastResult LineMap::Raycast( const Vec3& startPos, const Vec3& forwardNormal, float maxDist ) const
{
	RaycastResult result;
	result.startPos = startPos;
	result.forwardNormal = forwardNormal;
	result.maxDist = maxDist;

	return result;
}


//-----------------------------------------------------------------------------------------------
void LineMap::ResolveEntityVsWallCollisions()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		ResolveEntityVsWallCollision( *entity );
	}
}


//-----------------------------------------------------------------------------------------------
void LineMap::ResolveEntityVsWallCollision( Entity& entity )
{
	for ( int wallIdx = 0; wallIdx < (int)m_walls.size(); ++wallIdx )
	{
		Vec3 newPosition = entity.m_position + entity.m_collisionCenterOffset;
		PushSphereOutOfOBB3D( newPosition, entity.GetPhysicsRadius(), m_walls[wallIdx] );
		entity.m_position = newPosition - entity.m_collisionCenterOffset;
	}
}


//-----------------------------------------------------------------------------------------------
void LineMap::ApplyGravityToEntities()
{
	for ( int entityIdx = 0; entityIdx < (int)m_entities.size(); ++entityIdx )
	{
		Entity* const& entity = m_entities[entityIdx];
		if ( entity == nullptr )
		{
			continue;
		}

		entity->m_position += m_gravityVec * m_world->m_worldClock->GetLastDeltaSeconds();
	}
}

