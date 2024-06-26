#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Capsule2.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Math/Transform.hpp"
#include "Engine/Renderer/DebugRender.hpp"

#include <math.h>


//-----------------------------------------------------------------------------------------------
float ConvertDegreesToRadians( float angleDeg )
{
	return angleDeg / ( 180.f / fPI );
}


//-----------------------------------------------------------------------------------------------
float ConvertRadiansToDegrees( float angleRad )
{
	return angleRad * ( 180.f / fPI );
}


//-----------------------------------------------------------------------------------------------
float CosDegrees( float angleDeg )
{
	return cosf( ConvertDegreesToRadians( angleDeg ) );
}


//-----------------------------------------------------------------------------------------------
float SinDegrees( float angleDeg )
{
	return sinf( ConvertDegreesToRadians( angleDeg ) );
}


//-----------------------------------------------------------------------------------------------
float TanDegrees( float angleDeg )
{
	return tanf( ConvertDegreesToRadians( angleDeg ) );
}


//-----------------------------------------------------------------------------------------------
float Atan2Degrees( float y, float x )
{
	return ConvertRadiansToDegrees( atan2f( y, x ) );
}


//-----------------------------------------------------------------------------------------------
int PositiveMod( int left, int right )
{
	int modValue = left % right;

	if ( modValue < 0 )
	{
		modValue = right + modValue;
	}

	return modValue;
}


//-----------------------------------------------------------------------------------------------
float SignFloat( float value )
{
	return ( value >= 0.f ) ? 1.f : -1.f;
}


//-----------------------------------------------------------------------------------------------
bool IsNearlyEqual( float value, float target, float variance )
{
	if ( value > target - variance
		 && value < target + variance )
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool IsNearlyEqual( const Vec2& value, const Vec2& target, float variance /*= .0001f */ )
{
	if ( ( value.x > target.x - variance
		   && value.x < target.x + variance )
		 && ( value.y > target.y - variance
			  && value.y < target.y + variance ) )
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool IsNearlyEqual( const Vec3& value, const Vec3& target, float variance /*= .0001f */ )
{
	if ( ( value.x > target.x - variance
			&& value.x < target.x + variance )
		 && ( value.y > target.y - variance
			  && value.y < target.y + variance )
		 && ( value.z > target.z - variance
			  && value.z < target.z + variance ) )
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool IsNearlyEqual( const AABB2& value, const AABB2& target, float variance )
{
	return IsNearlyEqual( value.mins, target.mins, variance ) && IsNearlyEqual( value.maxs, target.maxs, variance );
}


//-----------------------------------------------------------------------------------------------
const Vec2 TransformPosition2D( const Vec2& initialPos, float uniformScale, float rotationDeg, const Vec2& translation )
{
	Vec2 transformedPos( initialPos );

	transformedPos *= uniformScale;
	transformedPos.RotateDegrees( rotationDeg );
	transformedPos += translation;

	return transformedPos;
}


//-----------------------------------------------------------------------------------------------
const Vec2 TransformPosition2D( const Vec2& initialPos, const Vec2& iBasis, const Vec2& jBasis, const Vec2& translation )
{
	Vec2 row1( iBasis.x, jBasis.x );
	Vec2 row2( iBasis.y, jBasis.y );
	
	float newX = DotProduct2D( initialPos, row1 );
	float newY = DotProduct2D( initialPos, row2 );

	return Vec2( newX, newY ) + translation;
}


//-----------------------------------------------------------------------------------------------
const Vec3 TransformPosition3DXY( const Vec3& initialPos, float uniformScale, float rotationDeg, const Vec2& translation )
{
	Vec2 transformedXYPos( initialPos.x, initialPos.y );

	transformedXYPos *= uniformScale;
	transformedXYPos.RotateDegrees( rotationDeg );
	transformedXYPos += translation;

	return Vec3( transformedXYPos.x,
				 transformedXYPos.y,
				 initialPos.z );
}


//-----------------------------------------------------------------------------------------------
const Vec3 TransformPosition3DXY( const Vec3& initialPos, const Vec2& iBasisXY, const Vec2& jBasisXY, const Vec2& translationXY )
{
	Vec2 transformedXYPos = TransformPosition2D( Vec2( initialPos.x, initialPos.y ), 
												 Vec2( iBasisXY.x, iBasisXY.y ),
												 Vec2( jBasisXY.x, jBasisXY.y ),
												 translationXY );

	return Vec3( transformedXYPos.x,
				 transformedXYPos.y,
				 initialPos.z );
}


//-----------------------------------------------------------------------------------------------
const Vec3 TransformPosition3DWithXYRotation( const Vec3& initialPos, float scaleUniform, float rotationDeg, const Vec3& translation )
{
	Vec3 transformedPos( initialPos );

	transformedPos *= scaleUniform;
	transformedPos.RotateAboutZDegrees( rotationDeg );
	transformedPos += translation;

	return transformedPos;
}


//-----------------------------------------------------------------------------------------------
float GetDistance2D( const Vec2& pos1, const Vec2& pos2 )
{
	return sqrtf( GetDistanceSquared2D(pos1, pos2) );
}


//-----------------------------------------------------------------------------------------------
float GetDistanceSquared2D( const Vec2& pos1, const Vec2& pos2 )
{
	float diffBetweenXCoords = pos2.x - pos1.x;
	float diffBetweenYCoords = pos2.y - pos1.y;

	return ( diffBetweenXCoords * diffBetweenXCoords ) + ( diffBetweenYCoords * diffBetweenYCoords ); 
}


//-----------------------------------------------------------------------------------------------
float GetDistance3D( const Vec3& pos1, const Vec3& pos2 )
{
	return sqrtf( GetDistanceSquared3D( pos1, pos2 ) );
}


//-----------------------------------------------------------------------------------------------
float GetDistanceSquared3D( const Vec3& pos1, const Vec3& pos2 )
{
	float diffBetweenXCoords = pos2.x - pos1.x;
	float diffBetweenYCoords = pos2.y - pos1.y;
	float diffBetweenZCoords = pos2.z - pos1.z;

	return ( diffBetweenXCoords * diffBetweenXCoords ) + ( diffBetweenYCoords * diffBetweenYCoords ) + ( diffBetweenZCoords * diffBetweenZCoords );
}


//-----------------------------------------------------------------------------------------------
float GetDistanceXY3D( const Vec3& pos1, const Vec3& pos2 )
{
	Vec2 pos1XY( pos1.x, pos1.y );
	Vec2 pos2XY( pos2.x, pos2.y );

	return GetDistance2D( pos1XY, pos2XY );
}


//-----------------------------------------------------------------------------------------------
float GetDistanceXYSquared3D( const Vec3& pos1, const Vec3& pos2 )
{
	Vec2 pos1XY( pos1.x, pos1.y );
	Vec2 pos2XY( pos2.x, pos2.y );

	return GetDistanceSquared2D( pos1XY, pos2XY );
}


//-----------------------------------------------------------------------------------------------
int GetTaxicabDistance2D( const IntVec2& positionA, const IntVec2& positionB )
{
	return abs( positionA.x - positionB.x ) + abs( positionA.y - positionB.y );
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetNormalizedDirectionFromAToB(const Vec2& a, const Vec2& b)
{
	Vec2 directionAToB = b - a;
	return directionAToB.GetNormalized();
}


//-----------------------------------------------------------------------------------------------
const Vec3 GetNormalizedDirectionFromAToB( const Vec3& a, const Vec3 b )
{
	Vec3 directionAToB = b - a;
	return directionAToB.GetNormalized();
}


//-----------------------------------------------------------------------------------------------
bool DoDiscsOverlap( const Vec2& center1, float radius1, const Vec2& center2, float radius2 )
{
	return GetDistance2D( center1, center2 ) < ( radius1 + radius2 );
}


//-----------------------------------------------------------------------------------------------
bool IsPointInsideSphere( const Vec3& point, const Vec3& center, float radius )
{
	float distSquared = GetDistanceSquared3D( center, point );
	float radiusSquared = radius * radius;

	return distSquared < radiusSquared;
}


//-----------------------------------------------------------------------------------------------
bool DoSpheresOverlap( const Vec3& center1, float radius1, const Vec3& center2, float radius2 )
{
	float sumOfRadiiSquared = ( radius1 + radius2 ) * ( radius1 + radius2 );
	return GetDistanceSquared3D( center1, center2 ) < sumOfRadiiSquared;
}


//-----------------------------------------------------------------------------------------------
bool DoSphereAndOBBOverlap3D( const Vec3& sphereCenter, float sphereRadius, const OBB3& obb )
{
	Vec3 nearestPointOnBox = obb.GetNearestPointOnBox( sphereCenter );
	float distFromCenterToBox = GetDistanceSquared3D( sphereCenter, nearestPointOnBox );
	
	return distFromCenterToBox < sphereRadius * sphereRadius;
}


//-----------------------------------------------------------------------------------------------
bool DoesRayIntersectPlane2D( const Vec2& rayStartPos, const Vec2& rayForwardNormal, const Plane2D& plane )
{
	if ( plane.IsPointInFront( rayStartPos ) )
	{
		// Check if entire ray is in front of plane
		return DotProduct2D( rayForwardNormal, plane.normal ) < 0.f;
	}
	else
	{
		// Check if entire ray is behind plane
		return DotProduct2D( rayForwardNormal, plane.normal ) > 0.f;
	}
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetRayIntersectionPointWithPlane2D( const Vec2& rayStartPos, const Vec2& rayForwardNormal, const Plane2D& plane )
{
	if ( !DoesRayIntersectPlane2D( rayStartPos, rayForwardNormal, plane ) )
	{
		return rayStartPos;
	}

	// Compute intersection point using similar triangles
	// ( rayForwardNormal dot plane.normal / 1 ) = ( dist of rayStartPos from plane / dist of rayStartPos to plane along rayForwardNormal )
	// t = dist of rayStartPos to plane along rayForwardNormal = ( dist of rayStartPos from plane along plane.normal / rayForwardNormal dot plane.normal ) 

	float fDotN = DotProduct2D( rayForwardNormal, plane.normal );
	float h = DotProduct2D( rayStartPos, plane.normal ) - plane.distance;
	float t = h / fDotN;
	   
	return rayStartPos - ( rayForwardNormal * t );
}


//-----------------------------------------------------------------------------------------------
float Interpolate( float a, float b, float fractionOfB )
{
	return a + fractionOfB * ( b - a );
}


//-----------------------------------------------------------------------------------------------
unsigned char Interpolate( unsigned char a, unsigned char b, float fractionOfB )
{
	return (unsigned char)( ( 1.f - fractionOfB ) * a ) + (unsigned char)( fractionOfB * b );
}


//-----------------------------------------------------------------------------------------------
Rgba8 InterpolateColor( const Rgba8& a, const Rgba8& b, float fractionOfB )
{
	// Don't quite get all the way to max to avoid wrapping color around
	Rgba8 lerpedColor;
	lerpedColor.r = Interpolate( a.r, b.r, fractionOfB  );
	lerpedColor.g = Interpolate( a.g, b.g, fractionOfB  );
	lerpedColor.b = Interpolate( a.b, b.b, fractionOfB  );
	lerpedColor.a = Interpolate( a.a, b.a, fractionOfB  );

	return lerpedColor;
}


//-----------------------------------------------------------------------------------------------
const Vec2 InterpolatePoint2D( const Vec2& a, const Vec2& b, float fractionOfB )
{
	// TODO: Optimize this
	Vec2 dir = GetNormalizedDirectionFromAToB( a, b );
	float length = GetDistance2D( a, b ) * fractionOfB;

	return a + ( dir * length );
}


//-----------------------------------------------------------------------------------------------
float RangeMapFloat( float inputBegin, float inputEnd, float outputBegin, float outputEnd, float inputValue )
{
	float inputDisplacement = inputValue - inputBegin;
	float inputRange = inputEnd - inputBegin;
	float fraction = inputDisplacement / inputRange;

	float outputRange = outputEnd - outputBegin;
	float outputDisplacement = fraction * outputRange;

	float outputValue = outputDisplacement + outputBegin;

	return outputValue;
}


//-----------------------------------------------------------------------------------------------
const Vec2 RangeMapFloatVec2( float inputBegin, float inputEnd, const Vec2& outputBegin, const Vec2& outputEnd, float value )
{
	float inputDomain = inputEnd - inputBegin;
	Vec2 outputRange = outputEnd - outputBegin;

	float fraction = ( value - inputBegin ) / inputDomain;

	return outputBegin + ( fraction * outputRange );
}


//-----------------------------------------------------------------------------------------------
const Vec3 RangeMapVec3( const Vec3& inputBegin, const Vec3& inputEnd, const Vec3& outputBegin, const Vec3& outputEnd, const Vec3& value )
{
	Vec3 newVec;
	newVec.x = RangeMapFloat( inputBegin.x, inputEnd.x, outputBegin.x, outputEnd.x, value.x );
	newVec.y = RangeMapFloat( inputBegin.y, inputEnd.y, outputBegin.y, outputEnd.y, value.y );
	newVec.z = RangeMapFloat( inputBegin.z, inputEnd.z, outputBegin.z, outputEnd.z, value.z );

	return newVec;
}


//-----------------------------------------------------------------------------------------------
int RoundDownToInt( float value )
{
	if ( value < 0.f )
	{
		--value;
	}

	return (int)value;
}


//-----------------------------------------------------------------------------------------------
float GetRotationInRangeDegrees( float rotationDegrees, float minAngleDegrees /*= -360.f*/, float maxAngleDegrees /*= 360.f */ )
{
	float newRotation = rotationDegrees;

	// Set between -360.f and 360.f
	while ( newRotation > 360.f ) newRotation -= 360.f;
	while ( newRotation < -360.f ) newRotation += 360.f;

	// Clamp within range
	if ( newRotation > maxAngleDegrees )
	{
		newRotation = minAngleDegrees + ( newRotation - maxAngleDegrees );
	}
	if ( newRotation < minAngleDegrees )
	{
		newRotation = maxAngleDegrees - ( minAngleDegrees - newRotation );
	}

	return newRotation;
}


//-----------------------------------------------------------------------------------------------
bool IsPointInsideDisc( const Vec2& point, const Vec2& center, float radius )
{
	return GetDistance2D( center, point ) < radius;
}


//-----------------------------------------------------------------------------------------------
bool IsPointInsideDiscFast( const Vec2& point, const Vec2& center, float radius )
{
	float distSquared = GetDistanceSquared2D( center, point );
	float radiusSquared = radius * radius;

	return distSquared < radiusSquared;
}


//-----------------------------------------------------------------------------------------------
bool IsPointInsideAABB2D( const Vec2& point, const AABB2& box )
{
	return box.IsPointInside( point );
}


//-----------------------------------------------------------------------------------------------
bool IsPointInsideCapsule2D( const Vec2& point, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius )
{
	if( IsPointInsideDisc( point, capsuleMidStart, capsuleRadius ) )
	{
		return true;
	}
	
	if ( IsPointInsideDisc( point, capsuleMidEnd, capsuleRadius ) )
	{
		return true;
	}

	Vec2 iBasis = capsuleMidEnd - capsuleMidStart;
	iBasis.Normalize();
	iBasis.Rotate90Degrees();
		
	Vec2 capsuleCenter = ( capsuleMidStart + capsuleMidEnd ) * .5f;
	Vec2 fullDimensions( capsuleRadius * 2.f, GetDistance2D( capsuleMidStart, capsuleMidEnd ) );

	OBB2 capsuleMiddleOBB2( capsuleCenter, fullDimensions, iBasis );

	return IsPointInsideOBB2D( point, capsuleMiddleOBB2 );
}


//-----------------------------------------------------------------------------------------------
bool IsPointInsideCapsule2D( const Vec2& point, const Capsule2& capsule )
{
	return capsule.IsPointInside( point );
}


//-----------------------------------------------------------------------------------------------
bool IsPointInsideOBB2D( const Vec2& point, const OBB2& box )
{
	return box.IsPointInside( point );
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnDisc2D( const Vec2& point, const Vec2& center, float radius )
{
	// The point is inside the disc
	if ( GetDistance2D( center, point ) < radius )
	{
		return point;
	}

	Vec2 directionToPoint = GetNormalizedDirectionFromAToB( center, point );
	return ( directionToPoint * radius ) + center;
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetNearestPointInsideDisc2D( const Vec2& point, const Vec2& center, float radius )
{
	// TODO: Figure out how this is different
	return GetNearestPointOnDisc2D( point, center, radius );
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnAABB2D( const Vec2& point, const AABB2& box )
{
	return box.GetNearestPoint( point );
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnInfiniteLine2D( const Vec2& point, const Vec2& pointOnLine, const Vec2& anotherPointOnLine )
{
	Vec2 lineDisplacement = anotherPointOnLine - pointOnLine;
	Vec2 localPoint = point - pointOnLine;

	Vec2 projectedPoint = GetProjectedOnto2D( localPoint, lineDisplacement );
	return projectedPoint + pointOnLine;
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnLineSegment2D( const Vec2& point, const Vec2& start, const Vec2& end )
{
	Vec2 se = end - start;
	Vec2 sp = point - start;
	Vec2 ep = point - end;
	
	// Region I
	if( DotProduct2D( se, sp ) < 0)
	{
		return start;
	}

	// Region II
	if ( DotProduct2D( se, ep ) > 0 )
	{
		return end;
	}

	// Region III
	return GetNearestPointOnInfiniteLine2D( point, start, end );
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnCapsule2D( const Vec2& point, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius )
{
	if ( IsPointInsideCapsule2D( point, capsuleMidStart, capsuleMidEnd, capsuleRadius ) )
	{
		return point;
	}

	Vec2 se = capsuleMidEnd - capsuleMidStart;
	Vec2 sp = point - capsuleMidStart;
	Vec2 ep = point - capsuleMidEnd;

	// Region I
	if ( DotProduct2D( se, sp ) < 0 )
	{
		return GetNearestPointOnDisc2D( point, capsuleMidStart, capsuleRadius );
	}

	// Region II
	if ( DotProduct2D( se, ep ) > 0 )
	{
		return GetNearestPointOnDisc2D( point, capsuleMidEnd, capsuleRadius );
	}

	// Region III
	Vec2 pointOnCapsuleMiddle = GetNearestPointOnInfiniteLine2D( point, capsuleMidStart, capsuleMidEnd );
	Vec2 displacementFromCapsuleMiddleToPoint = point - pointOnCapsuleMiddle;

	Vec2 pointOnCapsuleEdge = displacementFromCapsuleMiddleToPoint;
	pointOnCapsuleEdge.SetLength( capsuleRadius );

	return pointOnCapsuleEdge + pointOnCapsuleMiddle;
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnCapsule2D( const Vec2& point, const Capsule2& capsule )
{
	return capsule.GetNearestPoint( point );
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetNearestPointOnOBB2D( const Vec2& point, const OBB2& box )
{
	return box.GetNearestPoint( point );
}


//-----------------------------------------------------------------------------------------------
FloatRange GetRangeOnProjectedAxis( int numPoints, const Vec2* points, const Vec2& relativeToPos, const Vec2& axisNormal )
{
	FloatRange projectedRange( 999999999.f, -999999999.f ); 
	Vec2 normalizedAxis = axisNormal.GetNormalized();
	for ( int pointIndex = 0; pointIndex < numPoints; ++pointIndex )
	{
		float lengthAlongNormal = DotProduct2D( relativeToPos + points[pointIndex], normalizedAxis );

		if ( lengthAlongNormal > projectedRange.max )
		{
			projectedRange.max = lengthAlongNormal;
		}
		if ( lengthAlongNormal < projectedRange.min )
		{
			projectedRange.min = lengthAlongNormal;
		}
	}

	return projectedRange;
}


//-----------------------------------------------------------------------------------------------
bool DoLineSegmentAndDiscOverlap2D( const Vec2& lineStart, const Vec2& lineForwardNormal, float lineLength, const Vec2& discCenter, float discRadius )
{
	Vec2 lineStartToDiscCenter = discCenter - lineStart;

	float lineToDiscProjectedOntoNormal = DotProduct2D( lineForwardNormal, lineStartToDiscCenter );

	if ( lineLength < lineToDiscProjectedOntoNormal - discRadius
		 || lineToDiscProjectedOntoNormal + discRadius < 0.f )
	{
		return false;
	}

	Vec2 tangentToForward = lineForwardNormal.GetRotated90Degrees();

	float lineToDiscProjectedOntoTangent = DotProduct2D( tangentToForward, lineStartToDiscCenter );

	if ( lineToDiscProjectedOntoTangent > discRadius )
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool DoLineSegmentAndAABBOverlap2D( const Vec2& lineStart, const Vec2& lineForwardNormal, float lineLength, const AABB2& box )
{
	// Project each box point onto forward vector
	float forwardDotBottomLeft =	DotProduct2D( lineForwardNormal, box.mins - lineStart );
	float forwardDotBottomRight =	DotProduct2D( lineForwardNormal, Vec2( box.maxs.x, box.mins.y ) - lineStart );
	float forwardDotTopRight =		DotProduct2D( lineForwardNormal, box.maxs - lineStart );
	float forwardDotTopLeft =		DotProduct2D( lineForwardNormal, Vec2( box.mins.x, box.maxs.y ) - lineStart );

	// Check for entire box to the side of line
	if ( forwardDotBottomLeft > lineLength
		 && forwardDotBottomRight > lineLength
		 && forwardDotTopLeft > lineLength
		 && forwardDotTopRight > lineLength )
	{
		return false;
	}
	else if ( forwardDotBottomLeft < 0.f
		 && forwardDotBottomRight < 0.f
		 && forwardDotTopLeft < 0.f
		 && forwardDotTopRight < 0.f )
	{
		return false;
	}

	// Project each box point onto tangent to forward vector
	Vec2 tangentToForward = lineForwardNormal.GetRotated90Degrees();
	float tangentDotBottomLeft =	DotProduct2D( tangentToForward, box.mins - lineStart );
	float tangentDotBottomRight =	DotProduct2D( tangentToForward, Vec2( box.maxs.x, box.mins.y ) - lineStart );
	float tangentDotTopRight =		DotProduct2D( tangentToForward, box.maxs - lineStart );
	float tangentDotTopLeft =		DotProduct2D( tangentToForward, Vec2( box.mins.x, box.maxs.y ) - lineStart );

	// Check for entire box to the top or bottom of line
	if ( tangentDotBottomLeft > 0.f
		 && tangentDotBottomRight > 0.f
		 && tangentDotTopRight > 0.f
		 && tangentDotTopLeft > 0.f )
	{
		return false;
	}
	else if ( tangentDotBottomLeft < 0.f
			  && tangentDotBottomRight < 0.f
			  && tangentDotTopRight < 0.f
			  && tangentDotTopLeft < 0.f )
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool DoLineSegmentAndAABBOverlap2D( const Vec2& lineStart, const Vec2& lineEnd, const AABB2& box )
{
	Vec2 lineSegmentDisp = lineEnd - lineStart;
	Vec2 forwardNormal = lineSegmentDisp.GetNormalized();
	float length = lineSegmentDisp.GetLength();

	return DoLineSegmentAndAABBOverlap2D( lineStart, forwardNormal, length, box );
}


//-----------------------------------------------------------------------------------------------
bool DoOBBAndOBBOverlap2D( const OBB2& boxA, const OBB2& boxB )
{
	// Exit early if these are obviously not touching
	if ( !DoDiscsOverlap( boxA.GetCenter(), boxA.GetOuterRadius(), boxB.GetCenter(), boxB.GetOuterRadius() ) )
	{
		return false;
	}

	// Exit early if these are obviously touching
	if ( DoDiscsOverlap( boxA.GetCenter(), boxA.GetInnerRadius(), boxB.GetCenter(), boxB.GetInnerRadius() ) )
	{
		return true;
	}

	// The boxes are close enough that we need to do the full check
	Vec2 cornersA[4];
	boxA.GetCornerPositions( cornersA );

	Vec2 cornersB[4];
	boxB.GetCornerPositions( cornersB );

	// Check for overlapping projected ranges onto box A's i-basis
	FloatRange projectedA = GetRangeOnProjectedAxis( 4, cornersA, boxA.GetCenter(), boxA.m_iBasis );
	FloatRange projectedB = GetRangeOnProjectedAxis( 4, cornersB, boxA.GetCenter(), boxA.m_iBasis );

	if ( !projectedA.DoesOverlap( projectedB ) )
	{
		return false;
	}

	// Check for overlapping projected ranges onto box A's j-basis
	projectedA = GetRangeOnProjectedAxis( 4, cornersA, boxA.GetCenter(), boxA.GetJBasisNormal() );
	projectedB = GetRangeOnProjectedAxis( 4, cornersB, boxA.GetCenter(), boxA.GetJBasisNormal() );

	if ( !projectedA.DoesOverlap( projectedB ) )
	{
		return false;
	}

	// Check for overlapping projected ranges onto box B's i-basis
	projectedA = GetRangeOnProjectedAxis( 4, cornersA, boxB.GetCenter(), boxB.m_iBasis );
	projectedB = GetRangeOnProjectedAxis( 4, cornersB, boxB.GetCenter(), boxB.m_iBasis );

	if ( !projectedA.DoesOverlap( projectedB ) )
	{
		return false;
	}

	// Check for overlapping projected ranges onto box B's j-basis
	projectedA = GetRangeOnProjectedAxis( 4, cornersA, boxB.GetCenter(), boxB.GetJBasisNormal() );
	projectedB = GetRangeOnProjectedAxis( 4, cornersB, boxB.GetCenter(), boxB.GetJBasisNormal() );

	if ( !projectedA.DoesOverlap( projectedB ) )
	{
		return false;
	}

	// No axis of separation found
	return true;
}


//-----------------------------------------------------------------------------------------------
bool DoOBBAndAABBOverlap2D( const OBB2& obb, const AABB2& aabb )
{
	OBB2 aabb2AsObb2( aabb );

	return DoOBBAndOBBOverlap2D( obb, aabb2AsObb2 );
}


//-----------------------------------------------------------------------------------------------
bool DoOBBAndLineSegmentOverlap2D( const OBB2& obb, const Vec2& lineStart, const Vec2& lineEnd )
{
	// Exit early if these are obviously not touching
	Vec2 lineSegmentCenter = ( lineStart + lineEnd ) * .5f;
	float lineSegmentOuterRadius = ( lineSegmentCenter - lineStart ).GetLength();
	if ( !DoDiscsOverlap( obb.GetCenter(), obb.GetOuterRadius(), lineSegmentCenter, lineSegmentOuterRadius ) )
	{
		return false;
	}

	// The box and line segment are close enough that we need to do the full check
	Vec2 obbCorners[4];
	obb.GetCornerPositions( obbCorners );

	Vec2 lineSegmentPoints[] =
	{
		lineStart,
		lineEnd
	};
	
	Vec2 lineSegmentVector = lineEnd - lineStart;

	// Check for overlapping projected ranges onto line segment
	FloatRange projectedObb = GetRangeOnProjectedAxis( 4, obbCorners, lineSegmentCenter, lineSegmentVector );
	FloatRange projectedLineSegment = GetRangeOnProjectedAxis( 2, lineSegmentPoints, lineSegmentCenter, lineEnd - lineStart );

	if ( !projectedObb.DoesOverlap( projectedLineSegment ) )
	{
		return false;
	}

	// Check for overlapping projected ranges onto line segment
	Vec2 lineSegmentNormal( -lineSegmentVector.y, lineSegmentVector.x );
	projectedObb = GetRangeOnProjectedAxis( 4, obbCorners, lineSegmentCenter, lineSegmentNormal );
	projectedLineSegment = GetRangeOnProjectedAxis( 2, lineSegmentPoints, lineSegmentCenter, lineSegmentNormal );

	if ( !projectedObb.DoesOverlap( projectedLineSegment ) )
	{
		return false;
	}

	// Check for overlapping projected ranges onto obb's i-basis
	projectedObb = GetRangeOnProjectedAxis( 4, obbCorners, obb.GetCenter(), obb.m_iBasis );
	projectedLineSegment = GetRangeOnProjectedAxis( 2, lineSegmentPoints, obb.GetCenter(), obb.m_iBasis );

	if ( !projectedObb.DoesOverlap( projectedLineSegment ) )
	{
		return false;
	}

	// Check for overlapping projected ranges onto obb's j-basis
	projectedObb = GetRangeOnProjectedAxis( 4, obbCorners, obb.GetCenter(), obb.GetJBasisNormal() );
	projectedLineSegment = GetRangeOnProjectedAxis( 2, lineSegmentPoints, obb.GetCenter(), obb.GetJBasisNormal() );

	if ( !projectedObb.DoesOverlap( projectedLineSegment ) )
	{
		return false;
	}
	
	// No axis of separation found
	return true;
}


//-----------------------------------------------------------------------------------------------
bool DoOBBAndCapsuleOverlap2D( const OBB2& obb, const Vec2& capsuleMidStart, const Vec2& capsuleMidEnd, float capsuleRadius )
{
	if ( DoOBBAndDiscOverlap2D( obb, capsuleMidStart, capsuleRadius ) )
	{
		return true;
	}

	if ( DoOBBAndDiscOverlap2D( obb, capsuleMidEnd, capsuleRadius ) )
	{
		return true;
	}

	Vec2 capsuleCenter = ( capsuleMidStart + capsuleMidEnd ) * .5f;

	Vec2 capsuleMiddleSegment = capsuleMidEnd - capsuleMidStart;
	Vec2 fullDimensions( capsuleRadius * 2.f, capsuleMiddleSegment.GetLength() );

	Vec2 iBasis = capsuleMiddleSegment.GetNormalized().GetRotated90Degrees();

	OBB2 centerObb( capsuleCenter, fullDimensions, iBasis );

	if ( DoOBBAndOBBOverlap2D( obb, centerObb ) )
	{
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool DoOBBAndCapsuleOverlap2D( const OBB2& obb, const Capsule2& capsule )
{
	return DoOBBAndCapsuleOverlap2D( obb, capsule.m_middleStart, capsule.m_middleEnd, capsule.m_radius );
}


//-----------------------------------------------------------------------------------------------
bool DoOBBAndDiscOverlap2D( const OBB2& obb, const Vec2& discCenter, float discRadius )
{
	// Exit early if these are obviously not touching
	if ( !DoDiscsOverlap( obb.GetCenter(), obb.GetOuterRadius(), discCenter, discRadius ) )
	{
		return false;
	}

	// Exit early if these are obviously touching
	if ( DoDiscsOverlap( obb.GetCenter(), obb.GetInnerRadius(), discCenter, discRadius ) )
	{
		return true;
	}

	return IsPointInsideDisc( obb.GetNearestPoint( discCenter ), discCenter, discRadius );
}


//-----------------------------------------------------------------------------------------------
bool DoAABBsOverlap2D( const AABB2& box1, const AABB2& box2 )
{
	// Return the inverse of a check for if the boxes aren't overlapping
	bool boxesDontOverlap = (  box1.maxs.x < box2.mins.x
							|| box1.maxs.y < box2.mins.y
							|| box1.mins.x > box2.maxs.x
							|| box1.mins.y > box2.maxs.y );

	return !boxesDontOverlap;
}


//-----------------------------------------------------------------------------------------------
bool DoDiscAndAABBOverlap2D( const Vec2& center, float radius, const AABB2& box )
{
	// Return the inverse of a check for if the disc and box aren't overlapping
	bool discAndBoxDontOverlap = ( center.x + radius < box.mins.x
								|| center.y + radius < box.mins.y
								|| center.x - radius > box.maxs.x
								|| center.y - radius > box.maxs.y );

	return !discAndBoxDontOverlap;
}


//-----------------------------------------------------------------------------------------------
void PushDiscOutOfDisc2D( Vec2& mobileCenter, float mobileRadius, const Vec2& stationaryCenter, float stationaryRadius )
{
	if ( !DoDiscsOverlap( mobileCenter, mobileRadius, stationaryCenter, stationaryRadius ) )
	{
		return;
	}

	float overlapAmount = fabsf( ( mobileRadius + stationaryRadius ) - GetDistance2D( mobileCenter, stationaryCenter ) );
	Vec2 overlapCorrectionDirection = GetNormalizedDirectionFromAToB( stationaryCenter, mobileCenter );

	mobileCenter += ( overlapAmount * overlapCorrectionDirection );
}


//-----------------------------------------------------------------------------------------------
void PushDiscsOutOfEachOther2D( Vec2& center1, float radius1, Vec2& center2, float radius2 )
{
	if ( !DoDiscsOverlap( center1, radius1, center2, radius2 ) )
	{
		return;
	}

	float halfOverlapAmount = fabsf( ( radius1 + radius2 ) - GetDistance2D( center1, center2 ) ) * .5f;
	Vec2 overlapCorrectionDirection1 = GetNormalizedDirectionFromAToB( center2, center1 );
	Vec2 overlapCorrectionDirection2 = -overlapCorrectionDirection1;

	center1 += ( halfOverlapAmount * overlapCorrectionDirection1 );
	center2 += ( halfOverlapAmount * overlapCorrectionDirection2 );
}


//-----------------------------------------------------------------------------------------------
void PushDiscsOutOfEachOtherRelativeToMass2D( Vec2& center1, float radius1, float mass1, Vec2& center2, float radius2, float mass2 )
{
	GUARANTEE_OR_DIE( mass1 >= 0.f && mass2 >= 0.f, "Cannot call PushDiscsOutOfEachOtherRelativeToMass2D with a negative mass" );

	if ( !DoDiscsOverlap( center1, radius1, center2, radius2 ) )
	{
		return;
	}

	float fractionOfPushOnMass1 = 1.f;
	float fractionOfPushOnMass2 = 1.f;
	
	float overlapAmount = fabsf( ( radius1 + radius2 ) - GetDistance2D( center1, center2 ) );

	if ( IsNearlyEqual( mass1, mass2 ) )
	{
		fractionOfPushOnMass1 = .5f;
		fractionOfPushOnMass2 = .5f;
	}
	else if ( mass1 > mass2 )
	{
		fractionOfPushOnMass1 = mass2 / mass1;
		fractionOfPushOnMass2 = 1.f - fractionOfPushOnMass1;
	}
	else
	{
		fractionOfPushOnMass2 = mass1 / mass2;
		fractionOfPushOnMass1 = 1.f - fractionOfPushOnMass2;
	}

	Vec2 overlapCorrectionDirection1 = GetNormalizedDirectionFromAToB( center2, center1 );
	Vec2 overlapCorrectionDirection2 = -overlapCorrectionDirection1;

	center1 += ( fractionOfPushOnMass1 * overlapAmount * overlapCorrectionDirection1 );
	center2 += ( fractionOfPushOnMass2 * overlapAmount * overlapCorrectionDirection2 );
}

//-----------------------------------------------------------------------------------------------
void PushDiscOutOfPoint2D( Vec2& center, float radius, const Vec2& point )
{
	if ( !DoDiscsOverlap( center, radius, point, 0.f ) )
	{
		return;
	}

	float overlapAmount = fabsf( radius - GetDistance2D( center, point ) );
	Vec2 overlapCorrectionDirection = GetNormalizedDirectionFromAToB( point, center );

	center += ( overlapAmount * overlapCorrectionDirection );
}


//-----------------------------------------------------------------------------------------------
void PushDiscOutOfAABB2D( Vec2& center, float radius, const AABB2& box )
{
	if ( !DoDiscAndAABBOverlap2D( center, radius, box ) )
	{
		return;
	}

	Vec2 nearestPointOnAABB2 = GetNearestPointOnAABB2D( center, box );
	PushDiscOutOfPoint2D( center, radius, nearestPointOnAABB2 );
}


//-----------------------------------------------------------------------------------------------
float GetShortestAngularDisplacementDegrees( float orientationDegreesStart, float orientationDegreesEnd )
{
	// Adjust input angles to be between -360 and 360 degrees
	while ( orientationDegreesStart > 360.f )
	{
		orientationDegreesStart -= 360.f;
	}
	while ( orientationDegreesStart < -360.f )
	{
		orientationDegreesStart += 360.f;
	}

	while ( orientationDegreesEnd > 360.f )
	{
		orientationDegreesEnd -= 360.f;
	}
	while ( orientationDegreesEnd < -360.f )
	{
		orientationDegreesEnd += 360.f;
	}

	float angularDisplacementDegrees = orientationDegreesEnd - orientationDegreesStart;
	float angularDisplacementOtherWayDegrees = 360.f - fabsf( angularDisplacementDegrees );

	// Check if other way is shorter
	if ( fabsf( angularDisplacementDegrees ) > fabsf( angularDisplacementOtherWayDegrees ) )
	{
		if ( angularDisplacementDegrees > 0 )
		{
			return angularDisplacementOtherWayDegrees * -1.f;
		}
		else
		{
			return angularDisplacementOtherWayDegrees;
		}
	}

	return angularDisplacementDegrees;
}


//-----------------------------------------------------------------------------------------------
float GetTurnedToward( float currentOrientationDegrees, float goalOrientationDegrees, float deltaDegrees )
{
	float shortestAngle = GetShortestAngularDisplacementDegrees( currentOrientationDegrees, goalOrientationDegrees );

	if ( fabsf( shortestAngle ) < deltaDegrees )
	{
		return goalOrientationDegrees;
	}

	if ( shortestAngle < 0.f )
	{
		return currentOrientationDegrees - deltaDegrees;
	}

	return currentOrientationDegrees + deltaDegrees;
}


//-----------------------------------------------------------------------------------------------
float DotProduct2D( const Vec2& a, const Vec2& b )
{
	return ( a.x*b.x ) + ( a.y*b.y );
}


//-----------------------------------------------------------------------------------------------
float DotProduct3D( const Vec3& a, const Vec3& b )
{
	return ( a.x*b.x ) + ( a.y*b.y ) + ( a.z*b.z );
}


//-----------------------------------------------------------------------------------------------
float DotProduct4D( const Vec4& a, const Vec4& b )
{
	return ( a.x*b.x ) + ( a.y*b.y ) + ( a.z*b.z ) + ( a.w*b.w );
}


//-----------------------------------------------------------------------------------------------
float GetProjectedLength2D( const Vec2& a, const Vec2& b )
{
	if ( a == Vec2::ZERO
		 || b == Vec2::ZERO )
	{
		return 0;
	}

	Vec2 normalizedB = b.GetNormalized();
	float length = DotProduct2D( a, normalizedB );

	return length;
}


//-----------------------------------------------------------------------------------------------
float GetProjectedLength3D( const Vec3& a, const Vec3& b )
{
	if ( a == Vec3::ZERO
		 || b == Vec3::ZERO )
	{
		return 0;
	}

	Vec3 normalizedB = b.GetNormalized();
	float length = DotProduct3D( a, normalizedB );

	return length;
}


//-----------------------------------------------------------------------------------------------
const Vec2 GetProjectedOnto2D( const Vec2& a, const Vec2& onto )
{
	if ( a == Vec2::ZERO
		 || onto == Vec2::ZERO )
	{
		return Vec2::ZERO;
	}
	
	// Since we aren't normalizing onto, the dot product needs to be divided by the magnitude of onto,
	// then we need to divide onto by its length to get a unit vector in the direction of onto. This results in
	// ( A dot onto ) / onto.length / onto.length or / lengthSquared

	float dotResult = DotProduct2D( a, onto );
	float ontoLengthSquared = onto.GetLengthSquared();

	return ( dotResult * onto ) / ontoLengthSquared;
}


//-----------------------------------------------------------------------------------------------
const Vec3 GetProjectedOnto3D( const Vec3& a, const Vec3& onto )
{
	if ( a == Vec3::ZERO
		 || onto == Vec3::ZERO )
	{
		return Vec3::ZERO;
	}

	// Since we aren't normalizing onto, the dot product needs to be divided by the magnitude of onto,
	// then we need to divide onto by its length to get a unit vector in the direction of onto. This results in
	// ( A dot onto ) / onto.length / onto.length or / lengthSquared

	float dotResult = DotProduct3D( a, onto );
	float ontoLengthSquared = onto.GetLengthSquared();

	return ( dotResult * onto ) / ontoLengthSquared;
}


//-----------------------------------------------------------------------------------------------
float GetAngleDegreesBetweenVectors2D( const Vec2& a, const Vec2& b )
{
	return fabsf( GetShortestAngularDisplacementDegrees( a.GetOrientationDegrees(), b.GetOrientationDegrees() ) );
}


//-----------------------------------------------------------------------------------------------
Vec3 CrossProduct3D( const Vec3& a, const Vec3& b )
{
	Vec3 crossProduct;

	crossProduct.x = ( a.y * b.z ) - ( b.y * a.z );
	crossProduct.y = -( ( a.x * b.z ) - ( b.x * a.z ) );
	crossProduct.z = ( a.x * b.y ) - ( b.x * a.y );

	return crossProduct;
}


//-----------------------------------------------------------------------------------------------
Vec3 TripleProduct3D( const Vec3& a, const Vec3& b, const Vec3& c )
{
	Vec3 axb = CrossProduct3D( a, b );
	return CrossProduct3D( axb, c );
}


//-----------------------------------------------------------------------------------------------
Vec2 TripleProduct2D( const Vec2& a, const Vec2& b, const Vec2& c )
{
	Vec3 tripleProduct3D = TripleProduct3D( Vec3( a, 0.f ), Vec3( b, 0.f ), Vec3( c, 0.f ) );

	return Vec2( tripleProduct3D.x, tripleProduct3D.y );
}


//-----------------------------------------------------------------------------------------------
float SmoothStart2( float t )
{
	return t * t;
}


//-----------------------------------------------------------------------------------------------
float SmoothStart3( float t )
{
	return t * t * t;
}


//-----------------------------------------------------------------------------------------------
float SmoothStart4( float t )
{
	return t * t * t * t;
}


//-----------------------------------------------------------------------------------------------
float SmoothStart5( float t )
{
	return t * t * t * t * t;
}


//-----------------------------------------------------------------------------------------------
float SmoothStop2( float t )
{
	float oneMinusT = 1 - t;
	return 1.f - ( oneMinusT * oneMinusT );
}


//-----------------------------------------------------------------------------------------------
float SmoothStop3( float t )
{
	float oneMinusT = 1 - t;
	return 1.f - ( oneMinusT * oneMinusT * oneMinusT );
}


//-----------------------------------------------------------------------------------------------
float SmoothStop4( float t )
{
	float oneMinusT = 1 - t;
	return 1.f - ( oneMinusT * oneMinusT * oneMinusT * oneMinusT );
}


//-----------------------------------------------------------------------------------------------
float SmoothStop5( float t )
{
	float oneMinusT = 1 - t;
	return 1.f - ( oneMinusT * oneMinusT * oneMinusT * oneMinusT * oneMinusT );
}


//-----------------------------------------------------------------------------------------------
float SmoothStep3( float t )
{
	return ( 3 * ( t*t ) ) - ( 2 * ( t*t*t ) );
}


//-----------------------------------------------------------------------------------------------
bool IsPointInForwardSector2D( const Vec2& point, const Vec2& observerPos, float forwardDegrees, float apertureDegrees, float maxDist )
{
	Vec2 displacementToTarget = point - observerPos;
	if ( displacementToTarget.GetLengthSquared() > ( maxDist * maxDist ) )
	{
		return false;
	}

	float angleBetweenDegrees = GetAngleDegreesBetweenVectors2D( displacementToTarget, Vec2::MakeFromPolarDegrees( forwardDegrees ) );

	return angleBetweenDegrees < ( apertureDegrees * .5f );
}


////-----------------------------------------------------------------------------------------------
//void PushCylinderOutOfOBB3D( Vec3& cylinderBottomCenter, float cylinderRadius, float cylinderHeight, const OBB3& obb )
//{
//
//	/*if ( !DoCylinderAndOBBOverlap3D( cylinderBottomCenter, cylinderRadius, cylinderHeight, obb ) )
//	{
//		return;
//	}
//
//	DebugAddScreenText( Vec4::ZERO, Vec2::ZERO, 24.f, Rgba8::GREEN, Rgba8::GREEN, 0.f, "Overlap!" );*/
//	
//	// Sphere v sphere
//	Vec3 cylinderHeightVec = Transform::GetWorldUpVector() * cylinderHeight;
//	Vec3 cylinderCenter = cylinderBottomCenter + cylinderHeightVec * .5f;
//	Vec3 pushDirection = GetNormalizedDirectionFromAToB( obb.GetCenter(), cylinderCenter );
//
//
//	DebugAddWorldPoint( obb.GetNearestPointOnBox( cylinderCenter ), Rgba8::GREEN );
//
//	//Vec3 obbEdge = obb.GetFurthestPointInDirection( pushDirection );
//	/*Vec3 obbPoint = obb.GetCenter() + obb.GetOuterRadius() * pushDirection;
//	Vec3 cylinderPoint = cylinderCenter + ( ( cylinderHeight > cylinderRadius ? cylinderHeight : cylinderRadius ) * -pushDirection );
//	float pushDist = GetDistance3D( obbPoint, cylinderPoint );
//
//	cylinderBottomCenter += pushDist * pushDirection;*/
//}


//-----------------------------------------------------------------------------------------------
void PushSphereOutOfOBB3D( Vec3& sphereCenter, float sphereRadius, const OBB3& obb )
{
	if ( !DoSpheresOverlap( sphereCenter, sphereRadius,
							obb.GetCenter(), obb.GetOuterRadius() ) )
	{
		return;
	}

	Vec3 nearestPointOnBox = obb.GetNearestPointOnBox( sphereCenter );
	//DebugAddWorldPoint( nearestPointOnBox, Rgba8::ORANGE );

	Vec3 displacementFromBox = sphereCenter - nearestPointOnBox;
	Vec3 directionToPush = displacementFromBox.GetNormalized();
	float displacementLength = displacementFromBox.GetLength();
	float penetrationDist = sphereRadius - displacementLength;

	if ( penetrationDist > 0.f )
	{
		sphereCenter += penetrationDist * directionToPush;
	}
}


//-----------------------------------------------------------------------------------------------
float ClampMinMax( float value, float minimumValue, float maximumValue )
{
	if ( value < minimumValue )
	{
		return minimumValue;
	}
	else if ( value > maximumValue )
	{
		return maximumValue;
	}
	else
	{
		return value;
	}
}


//-----------------------------------------------------------------------------------------------
double ClampMinMax( double value, double minimumValue, double maximumValue )
{
	if ( value < minimumValue )
	{
		return minimumValue;
	}
	else if ( value > maximumValue )
	{
		return maximumValue;
	}
	else
	{
		return value;
	}
}


//-----------------------------------------------------------------------------------------------
unsigned char ClampMinMax( unsigned char value, unsigned char minimumValue, unsigned char maximumValue )
{
	if ( value < minimumValue )
	{
		return minimumValue;
	}
	else if ( value > maximumValue )
	{
		return maximumValue;
	}
	else
	{
		return value;
	}
}


//-----------------------------------------------------------------------------------------------
float ClampMin( float value, float minimumValue )
{
	if ( value < minimumValue )
	{
		return minimumValue;
	}
	else
	{
		return value;
	}
}


//-----------------------------------------------------------------------------------------------
float ClampZeroToOne( float value )
{
	return ClampMinMax( value, 0.f, 1.f );
}


//-----------------------------------------------------------------------------------------------
int ClampMinMaxInt( int value, int minimumValue, int maximumValue )
{
	if ( value < minimumValue )
	{
		return minimumValue;
	}
	else if ( value > maximumValue )
	{
		return maximumValue;
	}
	else
	{
		return value;
	}
}


//-----------------------------------------------------------------------------------------------
float SnapMinMax( float value, float minimumValue, float maximumValue )
{
	if ( value < minimumValue )
	{
		return minimumValue;
	}
	else if ( value > maximumValue )
	{
		return maximumValue;
	}
	else if( value >= ( minimumValue + maximumValue ) * .5f )
	{
		return maximumValue;
	}
	else
	{
		return minimumValue;
	}
}

