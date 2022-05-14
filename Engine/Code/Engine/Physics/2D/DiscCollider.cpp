#include "Engine/Physics/2D/DiscCollider.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/DebugRender.hpp"


//-----------------------------------------------------------------------------------------------
DiscCollider::DiscCollider( float radius, const Vec3& localPosition )
	:  m_radius( radius )
{
	m_type = COLLIDER_DISC;
	m_worldBounds = CalculateWorldBounds();
	m_localPosition = localPosition;
}


//-----------------------------------------------------------------------------------------------
void DiscCollider::UpdateWorldShape()
{
	m_worldPosition = m_localPosition;

	if ( m_rigidbody != nullptr )
	{
		m_worldPosition += m_rigidbody->GetWorldPosition();
	}

	m_worldBounds.SetCenter( m_worldPosition.XY() );
}


//-----------------------------------------------------------------------------------------------
const Vec3 DiscCollider::GetClosestPoint( const Vec3& pos ) const
{
	return Vec3( GetNearestPointOnDisc2D( pos.XY(), m_worldPosition.XY(), m_radius ), 0.f );
}


//-----------------------------------------------------------------------------------------------
bool DiscCollider::Contains( const Vec3& pos ) const
{
	return IsPointInsideDisc( pos.XY(), m_worldPosition.XY(), m_radius );
}


//-----------------------------------------------------------------------------------------------
//unsigned int DiscCollider::CheckIfOutsideScreen( const AABB2& screenBounds, bool checkForCompletelyOffScreen ) const
//{
//	unsigned int edges = SCREEN_EDGE_NONE;
//	
//	Vec2 discMins( m_worldPosition.x - m_radius, m_worldPosition.y - m_radius );
//	Vec2 discMaxs( m_worldPosition.x + m_radius, m_worldPosition.y + m_radius );
//
//	if ( checkForCompletelyOffScreen )
//	{
//		if ( screenBounds.mins.x > discMaxs.x )
//		{
//			edges |= SCREEN_EDGE_LEFT;
//		}
//		else if ( screenBounds.maxs.x < discMins.x )
//		{
//			edges |= SCREEN_EDGE_RIGHT;
//		}
//
//		if ( screenBounds.mins.y > discMaxs.y )
//		{
//			edges |= SCREEN_EDGE_BOTTOM;
//		}
//		else if ( screenBounds.maxs.y < discMins.y )
//		{
//			edges |= SCREEN_EDGE_TOP;
//		}
//	}
//	else
//	{
//		if ( screenBounds.mins.x > discMins.x )
//		{
//			edges |= SCREEN_EDGE_LEFT;
//		}
//		else if ( screenBounds.maxs.x < discMaxs.x )
//		{
//			edges |= SCREEN_EDGE_RIGHT;
//		}
//
//		if ( screenBounds.mins.y > discMins.y )
//		{
//			edges |= SCREEN_EDGE_BOTTOM;
//		}
//		else if ( screenBounds.maxs.y < discMaxs.y )
//		{
//			edges |= SCREEN_EDGE_TOP;
//		}
//	}
//
//	return edges;
//}


//-----------------------------------------------------------------------------------------------
const AABB2 DiscCollider::CalculateWorldBounds()
{
	Vec2 discMins( m_worldPosition.x - m_radius, m_worldPosition.y - m_radius );
	Vec2 discMaxs( m_worldPosition.x + m_radius, m_worldPosition.y + m_radius );

	AABB2 boundingBox( discMins, discMaxs );

	return boundingBox;
}


//-----------------------------------------------------------------------------------------------
float DiscCollider::CalculateMoment( float mass )
{
	return .5f * mass * m_radius * m_radius;
}


//-----------------------------------------------------------------------------------------------
Vec2 DiscCollider::GetFarthestPointInDirection( const Vec2& direction ) const
{
	return m_worldPosition.XY() + ( m_radius * direction.GetNormalized() );
}


//-----------------------------------------------------------------------------------------------
void DiscCollider::DebugRender( const Rgba8& borderColor, const Rgba8& fillColor ) const
{
	UNUSED( borderColor ); UNUSED( fillColor );
	/*DrawDisc2D( m_worldPosition, m_radius, fillColor );
	DrawRing2D( m_worldPosition, m_radius, borderColor, .04f );

	if ( m_rigidbody == nullptr )
	{
		return;
	}

	Vec2 endPoint = m_worldPosition + Vec2::MakeFromPolarRadians( m_rigidbody->GetOrientationRadians() ) * m_radius;
	DrawLine2D( m_worldPosition, endPoint, borderColor, .04f );*/
}
