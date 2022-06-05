#include "Engine/Math/OBB3.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Math/MathUtils.hpp"

#include <math.h>


//-----------------------------------------------------------------------------------------------
OBB3::OBB3( const OBB3& copyFrom )
	: m_center( copyFrom.m_center )
	, m_halfDimensions( copyFrom.m_halfDimensions )
	, m_iBasis( copyFrom.m_iBasis )
	, m_jBasis( copyFrom.m_jBasis )
{
}


//-----------------------------------------------------------------------------------------------
OBB3::OBB3( const Vec3& center, const Vec3& fullDimensions, const Vec3& iBasisNormal, const Vec3& jBasisNormal )
	: m_center( center )
	, m_halfDimensions( fullDimensions * .5f )
	, m_iBasis( iBasisNormal )
	, m_jBasis( jBasisNormal )
{
}


//-----------------------------------------------------------------------------------------------
void OBB3::SetFromText( const char* asText )
{
	ERROR_AND_DIE( Stringf( "Why are we trying to construct an OBB3 from text? '%s'", asText ) );
}


//-----------------------------------------------------------------------------------------------
std::string OBB3::GetAsString() const
{
	return ToString();
}


//-----------------------------------------------------------------------------------------------
std::string OBB3::ToString() const
{
	return Stringf( "%s %s %s %s", m_center.ToString().c_str(), m_halfDimensions.ToString().c_str(), m_iBasis.ToString().c_str(), m_jBasis.ToString().c_str() );
}


//-----------------------------------------------------------------------------------------------
const Vec3 OBB3::GetKBasisNormal() const
{
	Vec3 kBasis = CrossProduct3D( m_iBasis, m_jBasis );
	return kBasis.GetNormalized();
}


//-----------------------------------------------------------------------------------------------
void OBB3::GetCornerPositions( Vec3* out_eightPoints ) const
{
	Vec3 boxHalfWidth(	m_halfDimensions.x * m_iBasis );
	Vec3 boxHalfHeight( m_halfDimensions.y * m_jBasis );
	Vec3 boxHalfDepth(	m_halfDimensions.z * -GetKBasisNormal() );

	Vec3 topFrontRight(		m_center + boxHalfWidth + boxHalfHeight - boxHalfDepth );
	Vec3 topFrontLeft(		m_center - boxHalfWidth + boxHalfHeight - boxHalfDepth );
	Vec3 bottomFrontLeft(	m_center - boxHalfWidth - boxHalfHeight - boxHalfDepth );
	Vec3 bottomFrontRight(	m_center + boxHalfWidth - boxHalfHeight - boxHalfDepth );

	Vec3 topBackRight(		m_center + boxHalfWidth + boxHalfHeight + boxHalfDepth );
	Vec3 topBackLeft(		m_center - boxHalfWidth + boxHalfHeight + boxHalfDepth );
	Vec3 bottomBackLeft(	m_center - boxHalfWidth - boxHalfHeight + boxHalfDepth );
	Vec3 bottomBackRight(	m_center + boxHalfWidth - boxHalfHeight + boxHalfDepth );

	out_eightPoints[0] = bottomFrontLeft;
	out_eightPoints[1] = bottomFrontRight;
	out_eightPoints[2] = topFrontLeft;
	out_eightPoints[3] = topFrontRight;

	out_eightPoints[4] = bottomBackLeft;
	out_eightPoints[5] = bottomBackRight;
	out_eightPoints[6] = topBackLeft;
	out_eightPoints[7] = topBackRight;
}


//-----------------------------------------------------------------------------------------------
float OBB3::GetOuterRadius() const
{
	/*float radius = m_halfDimensions.x;
	if ( radius < m_halfDimensions.y ) { radius = m_halfDimensions.y; }
	if ( radius < m_halfDimensions.z ) { radius = m_halfDimensions.z; }*/

	Vec3 boxHalfWidth( m_halfDimensions.x * m_iBasis );
	Vec3 boxHalfHeight( m_halfDimensions.y * m_jBasis );
	Vec3 boxHalfDepth( m_halfDimensions.z * -GetKBasisNormal() );

	Vec3 topFrontRight( m_center + boxHalfWidth + boxHalfHeight - boxHalfDepth );
	Vec3 bottomBackLeft( m_center - boxHalfWidth - boxHalfHeight + boxHalfDepth );

	return GetDistance3D( topFrontRight, bottomBackLeft ) * .5f;
}


//-----------------------------------------------------------------------------------------------
const Vec3 OBB3::GetFurthestCornerInDirection( const Vec3& direction ) const
{
	Vec3 corners[8];
	GetCornerPositions( corners );
	Vec3 furthestPoint = corners[0];
	float maxDist = -9999.f;
	for ( int i = 0; i < 8; ++i )
	{
		float dist = DotProduct3D( direction, corners[i] );
		if ( dist > maxDist )
		{
			maxDist = dist;
			furthestPoint = corners[i];
		}
	}

	return furthestPoint;
}


//-----------------------------------------------------------------------------------------------
const Vec3 OBB3::GetNearestPoint( const Vec3& point ) const
{
	Vec3 pointIJK = point - m_center;

	float iLength = DotProduct3D( pointIJK, m_iBasis );
	float jLength = DotProduct3D( pointIJK, m_jBasis );
	float kLength = DotProduct3D( pointIJK, GetKBasisNormal() );

	Vec3 nearestPoint( ClampMinMax( iLength, -m_halfDimensions.x, m_halfDimensions.x ) * m_iBasis
					   + ClampMinMax( jLength, -m_halfDimensions.y, m_halfDimensions.y ) * m_jBasis
					   + ClampMinMax( kLength, -m_halfDimensions.z, m_halfDimensions.z ) * GetKBasisNormal() );

	nearestPoint += m_center;
	return nearestPoint;
}


//-----------------------------------------------------------------------------------------------
const Vec3 OBB3::GetNearestPointOnBox( const Vec3& point ) const
{
	Vec3 pointIJK = point - m_center;

	float iLength = DotProduct3D( pointIJK, m_iBasis );
	float jLength = DotProduct3D( pointIJK, m_jBasis );
	float kLength = DotProduct3D( pointIJK, GetKBasisNormal() );

	float iClampedLength = ClampMinMax( iLength, -m_halfDimensions.x, m_halfDimensions.x );
	float jClampedLength = ClampMinMax( jLength, -m_halfDimensions.y, m_halfDimensions.y );
	float kClampedLength = ClampMinMax( kLength, -m_halfDimensions.z, m_halfDimensions.z );

	// Adjust for point inside box
	//if ( IsNearlyEqual( iLength, iClampedLength, .00001f )
	//	 && IsNearlyEqual( jLength, jClampedLength, .00001f )
	//	 && IsNearlyEqual( kLength, kClampedLength, .00001f ) )
	//{
	//	float iOffsetFromInside = 0.f;
	//	if ( IsNearlyEqual( iLength, iClampedLength, .00001f ) )
	//	{
	//		if ( iLength > 0.f )
	//		{
	//			iOffsetFromInside = m_halfDimensions.x - iLength;
	//		}
	//		else
	//		{
	//			iOffsetFromInside = -m_halfDimensions.x + iLength;
	//		}
	//	}

	//	float jOffsetFromInside = 0.f;
	//	if ( IsNearlyEqual( jLength, jClampedLength, .00001f ) )
	//	{
	//		if ( jLength > 0.f )
	//		{
	//			jOffsetFromInside = m_halfDimensions.y - jLength;
	//		}
	//		else
	//		{
	//			jOffsetFromInside = -m_halfDimensions.y + jLength;
	//		}
	//	}

	//	float kOffsetFromInside = 0.f;
	//	if ( IsNearlyEqual( kLength, kClampedLength, .00001f ) )
	//	{
	//		if ( kLength > 0.f )
	//		{
	//			kOffsetFromInside = m_halfDimensions.z - kLength;
	//		}
	//		else
	//		{
	//			kOffsetFromInside = -m_halfDimensions.z + kLength;
	//		}
	//	}

	//	if ( !IsNearlyEqual( iOffsetFromInside, 0.f, .00001f )
	//		 && Abs( iOffsetFromInside ) < Abs( jOffsetFromInside )
	//		 && Abs( iOffsetFromInside ) < Abs( kOffsetFromInside ) )
	//	{
	//		// Need to snap i
	//		iClampedLength = SnapMinMax( iLength, -m_halfDimensions.x, m_halfDimensions.x );
	//	}
	//	else if ( !IsNearlyEqual( jOffsetFromInside, 0.f, .00001f )
	//			  && Abs( jOffsetFromInside ) < Abs( iOffsetFromInside )
	//			  && Abs( jOffsetFromInside ) < Abs( kOffsetFromInside ) )
	//	{
	//		// Need to snap j
	//		jClampedLength = SnapMinMax( jLength, -m_halfDimensions.y, m_halfDimensions.y );
	//	}
	//	else if ( !IsNearlyEqual( kOffsetFromInside, 0.f, .00001f ) )
	//	{
	//		// Need to snap k
	//		kClampedLength = SnapMinMax( kLength, -m_halfDimensions.z, m_halfDimensions.z );
	//	}
	//}

	Vec3 nearestPoint( iClampedLength * m_iBasis
					   + jClampedLength * m_jBasis
					   + kClampedLength * GetKBasisNormal() );

	nearestPoint += m_center;
	return nearestPoint;
}


//-----------------------------------------------------------------------------------------------
bool OBB3::IsPointInside( const Vec3& point ) const
{
	Vec3 pointIJK = point - m_center;

	float iLength = DotProduct3D( pointIJK, m_iBasis );
	if ( iLength > m_halfDimensions.x
		 || iLength < -m_halfDimensions.x )
	{
		return false;
	}

	float jLength = DotProduct3D( pointIJK, m_jBasis );
	if ( jLength > m_halfDimensions.y
		 || jLength < -m_halfDimensions.y )
	{
		return false;
	}

	float kLength = DotProduct3D( pointIJK, GetKBasisNormal() );
	if ( kLength > m_halfDimensions.z
		 || kLength < -m_halfDimensions.z )
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
void OBB3::Translate( const Vec3& translation )
{
	m_center += translation;
}


//-----------------------------------------------------------------------------------------------
void OBB3::SetCenter( const Vec3& newCenter )
{
	m_center = newCenter;
}


//-----------------------------------------------------------------------------------------------
void OBB3::SetDimensions( const Vec3& newDimensions )
{
	m_halfDimensions = newDimensions * .5f;
}


//-----------------------------------------------------------------------------------------------
void OBB3::Fix()
{
	// Ensure dimensions aren't negative
	m_halfDimensions.x = fabsf( m_halfDimensions.x );
	m_halfDimensions.y = fabsf( m_halfDimensions.y );
	m_halfDimensions.z = fabsf( m_halfDimensions.z );

	// Normalize i basis
	if ( m_iBasis == Vec3::ZERO )
	{
		m_iBasis = Vec3( 1.f, 0.f, 0.f );
	}
	else
	{
		m_iBasis.Normalize();
	}

	// Normalize j basis
	if ( m_jBasis == Vec3::ZERO )
	{
		m_jBasis = Vec3( 0.f, 1.f, 0.f );
	}
	else
	{
		m_jBasis.Normalize();
	}
}


//-----------------------------------------------------------------------------------------------
//const Vec2 OBB3::GetPointInLocalSpaceFromWorldSpace( const Vec3& point )
//{
//
//}


//-----------------------------------------------------------------------------------------------
void OBB3::operator=( const OBB3& assignFrom )
{
	m_center = assignFrom.m_center;
	m_halfDimensions = assignFrom.m_halfDimensions;
	m_iBasis = assignFrom.m_iBasis;
	m_jBasis = assignFrom.m_jBasis;
}
