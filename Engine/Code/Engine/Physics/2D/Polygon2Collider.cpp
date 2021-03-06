#include "Engine/Physics/2D/Polygon2Collider.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Physics/2D/DiscCollider.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
Polygon2Collider::Polygon2Collider()
{
	m_type = COLLIDER_POLYGON;
}


//-----------------------------------------------------------------------------------------------
void Polygon2Collider::UpdateWorldShape()
{
	m_worldPosition = m_localPosition;

	if ( m_rigidbody != nullptr )
	{
		m_worldPosition += m_rigidbody->GetWorldPosition();

		m_polygon.SetCenterOfMassAndUpdatePoints( m_worldPosition.XY() );

		m_polygon.SetOrientation( m_rigidbody->GetOrientationDegrees() );
	}
	else
	{
		m_polygon.SetCenterOfMassAndUpdatePoints( m_worldPosition.XY() );
	}

}


//-----------------------------------------------------------------------------------------------
const Vec3 Polygon2Collider::GetClosestPoint( const Vec3& pos ) const
{
	return Vec3( m_polygon.GetClosestPoint( pos.XY() ), 0.f );
}


//-----------------------------------------------------------------------------------------------
bool Polygon2Collider::Contains( const Vec3& pos ) const
{
	return m_polygon.Contains( pos.XY() );
}


//-----------------------------------------------------------------------------------------------
//unsigned int PolygonCollider2D::CheckIfOutsideScreen( const AABB2& screenBounds, bool checkForCompletelyOffScreen ) const
//{
//	unsigned int edges = SCREEN_EDGE_NONE;
//
//	const AABB2 polygonBoundingBox = GetWorldBounds();
//
//	if ( checkForCompletelyOffScreen )
//	{
//		if ( screenBounds.mins.x > polygonBoundingBox.maxs.x )
//		{
//			edges |= SCREEN_EDGE_LEFT;
//		}
//		else if ( screenBounds.maxs.x < polygonBoundingBox.mins.x )
//		{
//			edges |= SCREEN_EDGE_RIGHT;
//		}
//
//		if ( screenBounds.mins.y > polygonBoundingBox.maxs.y )
//		{
//			edges |= SCREEN_EDGE_BOTTOM;
//		}
//		else if ( screenBounds.maxs.y < polygonBoundingBox.mins.y )
//		{
//			edges |= SCREEN_EDGE_TOP;
//		}
//	}
//	else
//	{
//		if ( screenBounds.mins.x > polygonBoundingBox.mins.x )
//		{
//			edges |= SCREEN_EDGE_LEFT;
//		}
//		else if ( screenBounds.maxs.x < polygonBoundingBox.maxs.x )
//		{
//			edges |= SCREEN_EDGE_RIGHT;
//		}
//
//		if ( screenBounds.mins.y > polygonBoundingBox.mins.y )
//		{
//			edges |= SCREEN_EDGE_BOTTOM;
//		}
//		else if ( screenBounds.maxs.y < polygonBoundingBox.maxs.y )
//		{
//			edges |= SCREEN_EDGE_TOP;
//		}
//	}
//
//	return edges;
//}


//-----------------------------------------------------------------------------------------------
float Polygon2Collider::CalculateMoment( float mass )
{
	Vec2 v0 = m_polygon.m_points[0];
	float totalI = 0.f;
	float totalArea = 0.f;

	for ( int pointIdx = 1; pointIdx < (int)m_polygon.m_points.size() - 1; ++pointIdx )
	{
		Vec2 v1 = m_polygon.m_points[pointIdx];
		Vec2 v2 = m_polygon.m_points[pointIdx + 1];

		Vec2 u = v1 - v0;
		Vec2 v = v2 - v0;
		Vec2 center = ( v0 + v1 + v2 ) / 3.f;
		center -= m_worldPosition.XY();

		float uu = DotProduct2D( u, u );
		float vu = DotProduct2D( v, u );

		Vec2 h = v - ( ( vu / uu ) * u );

		float I = uu - vu + ( vu * vu / uu ) + DotProduct2D( h, h );

		// get area of triangle
		float area = .5f * u.GetLength() * h.GetLength();
		I *= ( area / 18.f );
		I += ( area * DotProduct2D( center, center ) );

		totalI += I;
		totalArea += area;
	}

	return ( mass / totalArea ) * totalI;
}


//-----------------------------------------------------------------------------------------------
Vec2 Polygon2Collider::GetFarthestPointInDirection( const Vec2& direction ) const
{
	const std::vector<Vec2>& points = m_polygon.GetPoints();

	float farthestDist = -INFINITY;
	Vec2 farthestPt( Vec2::ZERO );
	
	for ( int pointIdx = 0; pointIdx < (int)points.size(); ++pointIdx )
	{
		float distToPoint = DotProduct2D( points[pointIdx], direction );
		if ( distToPoint > farthestDist )
		{
			farthestDist = distToPoint;
			farthestPt = points[pointIdx];
		}
	}

	return farthestPt;
}


//-----------------------------------------------------------------------------------------------
void Polygon2Collider::DebugRender( const Rgba8& borderColor, const Rgba8& fillColor ) const
{
	UNUSED( borderColor ); UNUSED( fillColor );

	//DrawPolygon2( renderer, m_polygon.GetPoints(), fillColor );
	//DrawPolygon2Outline( renderer, m_polygon.GetPoints(), borderColor, .04f );

	/*Rgba8 boundingBoxColor = Rgba8::WHITE;
	boundingBoxColor.a = 100;
	DrawAABB2( renderer, m_polygon.m_boundingBox, boundingBoxColor );*/
}


//-----------------------------------------------------------------------------------------------
Collider* Polygon2Collider::Create( ColliderParams* params )
{
	Polygon2Collider* collider = new Polygon2Collider();
	collider->m_localPosition = params->GetValue( "localPosition", Vec3::ZERO );
	collider->m_polygon = params->GetValue( "polygon2", Polygon2() );

	return collider;
}

