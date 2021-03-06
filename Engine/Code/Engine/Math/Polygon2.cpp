#include "Engine/Math/Polygon2.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/ConvexHull2D.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/MathUtils.hpp"


//-----------------------------------------------------------------------------------------------
Polygon2::Polygon2( const std::vector<Vec2>& points )
	: m_points( points )
{
	CalculateBoundingBox();
	m_centerOfMass = m_boundingBox.GetCenter();
}


//-----------------------------------------------------------------------------------------------
Polygon2::~Polygon2()
{
	m_points.clear();
}


//-----------------------------------------------------------------------------------------------
void Polygon2::SetPoints( const std::vector<Vec2>& points )
{
	m_points.clear();

	m_points = points;

	CalculateBoundingBox();
	m_centerOfMass = m_boundingBox.GetCenter();
}


//-----------------------------------------------------------------------------------------------
void Polygon2::SetPoints( Vec2* points, int numPoints )
{
	m_points.clear();
	m_points.reserve( numPoints );

	for ( int pointIdx = 0; pointIdx < numPoints; ++pointIdx )
	{
		m_points.push_back( points[pointIdx] );
	}

	CalculateBoundingBox();
	m_centerOfMass = m_boundingBox.GetCenter();
}


//-----------------------------------------------------------------------------------------------
void Polygon2::SetFromText( const char* asText )
{
	ERROR_AND_DIE( Stringf( "Why are we trying to construct a polygon2 from text? '%s'", asText ) );
}


//-----------------------------------------------------------------------------------------------
std::string Polygon2::GetAsString() const
{
	return ToString();
}


//-----------------------------------------------------------------------------------------------
std::string Polygon2::ToString() const
{
	ERROR_AND_DIE( Stringf( "Why are we trying to serialize a polygon2 to text?" ) );
}


//-----------------------------------------------------------------------------------------------
bool Polygon2::IsValid() const
{
	if ( m_points.size() < 3 )
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool Polygon2::IsConvex() const
{
	if ( !IsValid() )
	{
		return false;
	}

	int totalNumPoints = (int)m_points.size();

	for ( int pointNumIdx = 0; pointNumIdx < totalNumPoints; ++pointNumIdx )
	{
		Vec2 pointA = m_points[pointNumIdx];
		Vec2 pointB;
		Vec2 pointC;
		if ( pointNumIdx == totalNumPoints - 1 )
		{
			pointB = m_points[0];
			pointC = m_points[1];
		}
		else if( pointNumIdx == totalNumPoints - 2 )
		{
			int lastPointIdx = totalNumPoints - 1;
			pointB = m_points[lastPointIdx];
			pointC = m_points[0];
		}
		else
		{
			int nextPointIdx = pointNumIdx + 1;
			pointB = m_points[nextPointIdx];
			
			++nextPointIdx;
			pointC = m_points[nextPointIdx];
		}

		Vec2 normal = pointB - pointA;
		normal.Rotate90Degrees();

		Vec2 nextEdge = pointC - pointA;

		// TODO: Handle edge cases discussed in class
		if ( DotProduct2D( nextEdge, normal ) < 0 )
		{
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool Polygon2::Contains( const Vec2& point ) const
{
	if ( !IsValid() )
	{
		return false;
	}

	int totalNumPoints = (int)m_points.size();

	for ( int pointNumIdx = 0; pointNumIdx < totalNumPoints; ++pointNumIdx )
	{
		Vec2 pointA = m_points[pointNumIdx];
		Vec2 pointB;
		if ( pointNumIdx == totalNumPoints - 1 )
		{
			pointB = m_points[0];
		}
		else
		{
			int nextPointIdx = pointNumIdx + 1;
			pointB = m_points[nextPointIdx];
		}

		Vec2 normal = pointB - pointA;
		normal.Rotate90Degrees();

		Vec2 nextEdge = point - pointB;

		// TODO: Handle edge cases discussed in class
		if ( DotProduct2D( nextEdge, normal ) < 0 )
		{
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
float Polygon2::GetDistance( const Vec2& point ) const
{
	if ( Contains( point ) )
	{
		return 0.f;
	}

	float minDistToPoint = 99999999.f;
	for ( int edgeIdx = 0; edgeIdx < GetEdgeCount(); ++edgeIdx )
	{
		Vec2 startVert;
		Vec2 endVert;
		GetEdge( edgeIdx, &startVert, &endVert );

		Vec2 nearestLocalPoint = GetNearestPointOnLineSegment2D( point, startVert, endVert );
		float distToPoint = GetDistance2D( nearestLocalPoint, point );

		if ( distToPoint < minDistToPoint )
		{
			minDistToPoint = distToPoint;
		}
	}

	return minDistToPoint;
}


//-----------------------------------------------------------------------------------------------
Vec2 Polygon2::GetClosestPoint( const Vec2& point ) const
{
	if ( Contains( point ) )
	{
		return point;
	}

	return GetClosestPointOnEdge( point );
}


//-----------------------------------------------------------------------------------------------
Vec2 Polygon2::GetClosestPointOnEdge( const Vec2& point ) const
{
	Vec2 nearestPoint = Vec2::ZERO;
	float minDistToPoint = 99999999.f;
	for ( int edgeIdx = 0; edgeIdx < GetEdgeCount(); ++edgeIdx )
	{
		Vec2 startVert;
		Vec2 endVert;
		GetEdge( edgeIdx, &startVert, &endVert );

		Vec2 nearestLocalPoint = GetNearestPointOnLineSegment2D( point, startVert, endVert );
		float distToPoint = GetDistance2D( nearestLocalPoint, point );

		if ( distToPoint < minDistToPoint )
		{
			minDistToPoint = distToPoint;
			nearestPoint = nearestLocalPoint;
		}
	}

	return nearestPoint;
}


//-----------------------------------------------------------------------------------------------
void Polygon2::GetClosestEdge( const Vec2& point, Vec2* out_start, Vec2* out_end ) const
{
	float minDistToPoint = 99999999.f;
	for ( int edgeIdx = 0; edgeIdx < GetEdgeCount(); ++edgeIdx )
	{
		Vec2 startVert;
		Vec2 endVert;
		GetEdge( edgeIdx, &startVert, &endVert );
		Vec2 edge = endVert - startVert;

		Vec2 normal = edge.GetRotatedMinus90Degrees().GetNormalized();

		float distToPoint = DotProduct2D( normal, startVert - point );

		if ( distToPoint < minDistToPoint )
		{
			minDistToPoint = distToPoint;
			*out_start = startVert;
			*out_end = endVert;
		}
	}
}


//-----------------------------------------------------------------------------------------------
ConvexHull2D Polygon2::GenerateConvexHull() const
{
	ConvexHull2D convexHull;

	for ( int pointNumIdx = 0; pointNumIdx < GetVertexCount(); ++pointNumIdx )
	{
		// Special case to handle wrapping around with final point
		if ( pointNumIdx == GetVertexCount() - 1 )
		{
			convexHull.AddPlane( GenerateOutwardFacingPlaneFromEdge( m_points[pointNumIdx], m_points[0] ) );
		}
		else
		{
			convexHull.AddPlane( GenerateOutwardFacingPlaneFromEdge( m_points[pointNumIdx], m_points[pointNumIdx + 1] ) );
		}
	}

	return convexHull;
}


//-----------------------------------------------------------------------------------------------
int Polygon2::GetVertexCount() const
{
	return (int)m_points.size();
}


//-----------------------------------------------------------------------------------------------
int Polygon2::GetEdgeCount() const
{
	return (int)m_points.size();
}


//-----------------------------------------------------------------------------------------------
void Polygon2::GetEdge( int edgeIndex, Vec2* out_start, Vec2* out_end ) const
{
	if ( edgeIndex >= GetEdgeCount() 
		 || edgeIndex < 0 )
	{
		out_start = nullptr;
		out_end = nullptr;
		return;
	}

	int firstVertIdx = edgeIndex - 1;
	int secondVertIdx = edgeIndex;

	if ( firstVertIdx == -1 )
	{
		firstVertIdx = GetVertexCount() - 1;
	}

	if ( secondVertIdx == GetVertexCount() )
	{
		secondVertIdx = 0;
	}

	*out_start = m_points[firstVertIdx];
	*out_end = m_points[secondVertIdx];
}


//-----------------------------------------------------------------------------------------------
void Polygon2::Translate2D( const Vec2& translation )
{
	for ( int pointNumIdx = 0; pointNumIdx < GetVertexCount(); ++pointNumIdx )
	{
		m_points[pointNumIdx] += translation;
	}
}


//-----------------------------------------------------------------------------------------------
void Polygon2::TranslateWithBoundingBox2D( const Vec2& translation )
{
	Translate2D( translation );
	m_boundingBox.Translate( translation );
}


//-----------------------------------------------------------------------------------------------
void Polygon2::Rotate2D( float rotationDegrees )
{
	for ( int pointNumIdx = 0; pointNumIdx < GetVertexCount(); ++pointNumIdx )
	{
		Vec2 translatedPoint = m_points[pointNumIdx] - GetCenterOfMass();
		translatedPoint.RotateDegrees( -rotationDegrees );
		translatedPoint += GetCenterOfMass();

		m_points[pointNumIdx] = translatedPoint;
	}
}


//-----------------------------------------------------------------------------------------------
void Polygon2::SetOrientation( float newOrientationDegrees )
{
	if ( IsNearlyEqual( newOrientationDegrees, m_orientationDegrees ) )
	{
		return;
	}

	float deltaDegrees = m_orientationDegrees - newOrientationDegrees;

	Rotate2D( deltaDegrees );
	CalculateBoundingBox();

	m_orientationDegrees = newOrientationDegrees;
}


//-----------------------------------------------------------------------------------------------
// TODO: Calculate more accurately
Vec2 Polygon2::GetCenterOfMass() const
{
	return m_centerOfMass;
}


//-----------------------------------------------------------------------------------------------
void Polygon2::SetCenterOfMassAndUpdatePoints( const Vec2& newCenterOfMass )
{
	Vec2 oldCenterOfMass = GetCenterOfMass();

	Vec2 translation = newCenterOfMass - oldCenterOfMass;

	Translate2D( translation );

	m_centerOfMass = newCenterOfMass;

	m_boundingBox.Translate( translation );
}


//-----------------------------------------------------------------------------------------------
void Polygon2::RotateAboutPoint2D( float rotationDegrees, const Vec2& rotationPoint )
{
	for ( int pointNumIdx = 0; pointNumIdx < GetVertexCount(); ++pointNumIdx )
	{
		Vec2 translatedPoint = m_points[pointNumIdx] - rotationPoint;
		translatedPoint.RotateDegrees( -rotationDegrees );
		translatedPoint += rotationPoint;

		m_points[pointNumIdx] = translatedPoint;
	}
}


//-----------------------------------------------------------------------------------------------
void Polygon2::ScaleAboutPoint2D( float scaleFactor, const Vec2& scaleOriginPoint )
{
	for ( int pointNumIdx = 0; pointNumIdx < GetVertexCount(); ++pointNumIdx )
	{
		Vec2 translatedPoint = m_points[pointNumIdx] - scaleOriginPoint;
		translatedPoint.SetLength( translatedPoint.GetLength() * scaleFactor );
		translatedPoint += scaleOriginPoint;

		m_points[pointNumIdx] = translatedPoint;
	}
}


//-----------------------------------------------------------------------------------------------
Plane2D Polygon2::GenerateOutwardFacingPlaneFromEdge( const Vec2& startPoint, const Vec2& endPoint ) const
{
	Vec2 displacement = endPoint - startPoint;
	Vec2 planeNormal = displacement.GetNormalized();
	planeNormal.RotateMinus90Degrees();

	return Plane2D( planeNormal, startPoint );
}


//-----------------------------------------------------------------------------------------------
void Polygon2::CalculateBoundingBox()
{
	// Initialize with first point in polygon
	AABB2 boundingBox( m_points[0], m_points[0] );

	for ( int pointIdx = 1; pointIdx < (int)m_points.size(); ++pointIdx )
	{
		boundingBox.StretchToIncludePoint( m_points[pointIdx] );
	}

	m_boundingBox = boundingBox;
}
