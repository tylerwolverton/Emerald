#include "Engine/Physics/Collider.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Physics/Manifold.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Renderer/DebugRender.hpp"


typedef bool ( *CollisionCheckCallback )( const Collider*, const Collider* );

int Collider::s_nextId = 1000;

//
////-----------------------------------------------------------------------------------------------
//static bool DiscVDiscCollisionCheck( const Collider* collider1, const Collider* collider2 )
//{
//	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
//	const DiscCollider2D* discCollider1 = (const DiscCollider2D*)collider1;
//	const DiscCollider2D* discCollider2 = (const DiscCollider2D*)collider2;
//
//	return DoDiscsOverlap( discCollider1->m_worldPosition, 
//						   discCollider1->m_radius, 
//						   discCollider2->m_worldPosition,
//						   discCollider2->m_radius );
//}
//
//
////-----------------------------------------------------------------------------------------------
//static bool DiscVPolygonCollisionCheck( const Collider* collider1, const Collider* collider2 )
//{
//	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
//	const DiscCollider2D* discCollider = (const DiscCollider2D*)collider1;
//	const PolygonCollider2D* polygonCollider = (const PolygonCollider2D*)collider2;
//
//	Vec2 nearestPoint = polygonCollider->GetClosestPoint( discCollider->m_worldPosition );
//
//	return IsPointInsideDisc( nearestPoint, discCollider->m_worldPosition, discCollider->m_radius );
//}
//
//
////-----------------------------------------------------------------------------------------------
//static Vec2 GetSupportPoint( const PolygonCollider2D* polygonCollider1, const PolygonCollider2D* polygonCollider2, const Vec2& direction )
//{
//	return polygonCollider1->GetFarthestPointInDirection( direction ) - polygonCollider2->GetFarthestPointInDirection( -direction );
//}
//
//
////-----------------------------------------------------------------------------------------------
//static std::vector<Vec2> GetSimplexForGJKCollision( const PolygonCollider2D* polygonCollider1, const PolygonCollider2D* polygonCollider2 )
//{
//	// Initial point calculation
//	Vec2 direction = polygonCollider2->m_worldPosition - polygonCollider1->m_worldPosition;
//	Vec2 supportPoint0 = GetSupportPoint( polygonCollider1, polygonCollider2, direction );
//	Vec2 supportPoint1 = GetSupportPoint( polygonCollider1, polygonCollider2, -direction );
//
//	Vec2 supportEdge01 = supportPoint1 - supportPoint0;
//	direction = TripleProduct2D( supportEdge01, -supportPoint0, supportEdge01 );
//
//	while ( true )
//	{
//		Vec2 supportPoint2 = GetSupportPoint( polygonCollider1, polygonCollider2, direction );
//
//		// If the new support point equals an existing one, we've hit the edge of the polygon so we know there is no intersection
//		if ( supportPoint2 == supportPoint0
//			 || supportPoint2 == supportPoint1 )
//		{
//			return std::vector<Vec2>{};
//		}
//
//		// Maintain ccw order
//		if ( supportEdge01.GetRotated90Degrees().GetNormalized() != direction.GetNormalized() )
//		{
//			Vec2 temp = supportPoint0;
//			supportPoint0 = supportPoint1;
//			supportPoint1 = temp;
//			supportEdge01 = supportPoint1 - supportPoint0;
//		}
//
//		// Get orthogonal vectors to each of the remaining edges to test for the origin
//		Vec2 supportPoint2ToOrigin = -supportPoint2;
//		Vec2 supportEdge21 = supportPoint1 - supportPoint2;
//		Vec2 supportEdge20 = supportPoint0 - supportPoint2;
//
//		Vec2 orthogonal21 = TripleProduct2D( supportEdge20, supportEdge21, supportEdge21 );
//		Vec2 orthogonal20 = TripleProduct2D( supportEdge21, supportEdge20, supportEdge20 );
//
//		// If origin is on opposite side of an edge, reset support points and try again
//		if ( DotProduct2D( orthogonal21, supportPoint2ToOrigin ) > 0.f )
//		{
//			supportPoint0 = supportPoint1;
//			supportPoint1 = supportPoint2;
//			direction = orthogonal21;
//			continue;
//		}
//
//		if ( DotProduct2D( orthogonal20, supportPoint2ToOrigin ) > 0.f )
//		{
//			supportPoint1 = supportPoint0;
//			supportPoint0 = supportPoint2;
//			direction = orthogonal20;
//			continue;
//		}
//		
//		// Make sure simplex is in ccw order
//		if ( orthogonal21.GetNormalized() != supportEdge21.GetRotated90Degrees().GetNormalized() )
//		{
//			return std::vector<Vec2>{ supportPoint0, supportPoint1, supportPoint2 };
//		}
//
//		return std::vector<Vec2>{ supportPoint2, supportPoint1, supportPoint0 };
//	}
//
//	return std::vector<Vec2>{};
//}
//
//
////-----------------------------------------------------------------------------------------------
//static bool PolygonVPolygonCollisionCheck( const Collider* collider1, const Collider* collider2 )
//{
//	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
//	const PolygonCollider2D* polygonCollider1 = (const PolygonCollider2D*)collider1;
//	const PolygonCollider2D* polygonCollider2 = (const PolygonCollider2D*)collider2;
//
//	std::vector<Vec2> simplex = GetSimplexForGJKCollision( polygonCollider1, polygonCollider2 );
//	if ( simplex.size() == 0 )
//	{
//		return false;
//	}
//
//	return true;
//}
//
//
////-----------------------------------------------------------------------------------------------
//// a "matrix" lookup is just a 2D array
//static CollisionCheckCallback g_CollisionChecks[NUM_COLLIDER_TYPES * NUM_COLLIDER_TYPES] = {
//	/*             disc,                         polygon, */
//	/*    disc */  DiscVDiscCollisionCheck,      nullptr,
//	/* polygon */  DiscVPolygonCollisionCheck,   PolygonVPolygonCollisionCheck
//};
//
//
////-----------------------------------------------------------------------------------------------
//void Collider::ChangeFriction( float deltaFriction )
//{
//	m_material.m_friction += deltaFriction;
//
//	m_material.m_friction = ClampZeroToOne( m_material.m_friction );
//}
//
//
////-----------------------------------------------------------------------------------------------
//bool Collider::Intersects( const Collider* other ) const
//{
//	if ( other == nullptr
//		 || !m_rigidbody->IsEnabled()
//		 || !other->m_rigidbody->IsEnabled()
//		 || !DoAABBsOverlap2D( GetWorldBounds(), other->GetWorldBounds() ) )
//	{
//		return false;
//	}
//
//	eCollider2DType myType = m_type;
//	eCollider2DType otherType = other->m_type;
//
//	if ( myType <= otherType ) 
//	{
//		int idx = otherType * NUM_COLLIDER_TYPES + myType;
//		CollisionCheckCallback callback = g_CollisionChecks[idx];
//		return callback( this, other );
//	}
//	else 
//	{
//		// flip the types when looking into the index.
//		int idx = myType * NUM_COLLIDER_TYPES + otherType;
//		CollisionCheckCallback callback = g_CollisionChecks[idx];
//		return callback( other, this );
//	}
//}
//
//
////-----------------------------------------------------------------------------------------------
//static Manifold2D DiscVDiscCollisionManifoldGenerator( const Collider* collider1, const Collider* collider2 )
//{
//	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
//	const DiscCollider2D* discCollider1 = (const DiscCollider2D*)collider1;
//	const DiscCollider2D* discCollider2 = (const DiscCollider2D*)collider2;
//	
//	Manifold2D manifold;
//	manifold.normal = discCollider2->m_worldPosition - discCollider1->m_worldPosition;
//	manifold.normal.Normalize();
//
//	Vec2 disc1Edge = discCollider1->m_worldPosition + ( manifold.normal * discCollider1->m_radius );
//	Vec2 disc2Edge = discCollider2->m_worldPosition + ( -manifold.normal * discCollider2->m_radius );
//	manifold.penetrationDepth = GetDistance2D( disc1Edge, disc2Edge );
//
//	manifold.contactPoint1 = disc1Edge - ( manifold.normal * manifold.penetrationDepth * .5f );
//	manifold.contactPoint2 = manifold.contactPoint1;
//
//	return manifold;
//}
//
//
////-----------------------------------------------------------------------------------------------
//static Manifold2D DiscVPolygonCollisionManifoldGenerator( const Collider* collider1, const Collider* collider2 )
//{
//	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
//	const DiscCollider2D* discCollider = (const DiscCollider2D*)collider1;
//	const PolygonCollider2D* polygonCollider = (const PolygonCollider2D*)collider2;
//
//	Vec2 closestPointOnPolygonToDisc = polygonCollider->m_polygon.GetClosestPointOnEdge( discCollider->m_worldPosition );
//	
//	Manifold2D manifold;
//	manifold.normal = closestPointOnPolygonToDisc - discCollider->m_worldPosition;
//	manifold.normal.Normalize();
//
//	// If disc is inside polygon flip the normal to ensure it is pushed out
//	if ( polygonCollider->m_polygon.Contains( discCollider->m_worldPosition ) )
//	{
//		manifold.normal *= -1.f;
//	}
//
//	Vec2 closestPointOnDiscToPolygon = discCollider->m_worldPosition + ( manifold.normal * discCollider->m_radius );
//	manifold.penetrationDepth = GetDistance2D( closestPointOnDiscToPolygon, closestPointOnPolygonToDisc );
//
//	manifold.contactPoint1 = closestPointOnDiscToPolygon - ( manifold.normal * manifold.penetrationDepth * .5f );
//	manifold.contactPoint2 = manifold.contactPoint1;
//
//	return manifold;
//}
//
//
////-----------------------------------------------------------------------------------------------
//static bool GetClippedSegmentToSegment( const Vec2& segmentToClipStart, const Vec2& segmentToClipEnd,
//										const Vec2& refEdgeStart, const Vec2& refEdgeEnd,
//										Vec2* out_clippedMin, Vec2* out_clippedMax )
//{
//	Vec2 refDir = ( refEdgeEnd - refEdgeStart ).GetNormalized();
//
//	float minDistAlongRefEdge = DotProduct2D( refEdgeStart, refDir );
//	float maxDistAlongRefEdge = DotProduct2D( refEdgeEnd, refDir );
//
//	float minDistAlongSegToClip = DotProduct2D( segmentToClipStart, refDir );
//	float maxDistAlongSegToClip = DotProduct2D( segmentToClipEnd, refDir );
//
//	float minClippedDist = Max( minDistAlongRefEdge, minDistAlongSegToClip );
//	float maxClippedDist = Min( maxDistAlongRefEdge, maxDistAlongSegToClip );
//		
//	// Check for no intersection
//	if ( minClippedDist > maxClippedDist )
//	{
//		return false;
//	}
//
//	*out_clippedMin = RangeMapFloatVec2( minDistAlongSegToClip, maxDistAlongSegToClip, segmentToClipStart, segmentToClipEnd, minClippedDist );
//	*out_clippedMax = RangeMapFloatVec2( minDistAlongSegToClip, maxDistAlongSegToClip, segmentToClipStart, segmentToClipEnd, maxClippedDist );
//	
//	return true;
//}
//
//
////-----------------------------------------------------------------------------------------------
//static void GetContactEdgeBetweenPolygons( const PolygonCollider2D* polygonCollider1, const PolygonCollider2D* polygonCollider2,
//										   const Vec2& normal, float penetrationDepth,
//										   Vec2* out_contactMin, Vec2* out_contactMax )
//{
//	Vec2 pointOnB = polygonCollider2->GetFarthestPointInDirection( normal );
//	Plane2D referencePlane( normal, pointOnB );
//
//	// Find every point of polygon2 that lies within a tolerance of the reference plane
//	const std::vector<Vec2>& pointsOfB = polygonCollider2->m_polygon.GetPoints();
//	std::vector<Vec2> pointsAlongReferencePlane;
//	for ( int pointIdx = 0; pointIdx < (int)pointsOfB.size(); ++pointIdx )
//	{
//		float distFromPlane = DotProduct2D( pointsOfB[pointIdx], normal ) - referencePlane.distance;
//		if ( fabsf( distFromPlane ) < .001f )
//		{
//			pointsAlongReferencePlane.push_back( pointsOfB[pointIdx] );
//		}
//	}
//
//	// Get the max and min values along the tangent to the normal, or the reference plane itself. 
//	// This gives the bounds of a reference edge ( or segment ) to check against
//	Vec2 tangent = normal.GetRotatedMinus90Degrees();
//	float maxDistAlongTangent = -INFINITY;
//	float minDistAlongTangent = INFINITY;
//	for ( int pointIdx = 0; pointIdx < (int)pointsAlongReferencePlane.size(); ++pointIdx )
//	{
//		float distAlongTangent = DotProduct2D( pointsAlongReferencePlane[pointIdx], tangent );
//		if ( distAlongTangent > maxDistAlongTangent )
//		{
//			maxDistAlongTangent = distAlongTangent;
//		}
//		if ( distAlongTangent < minDistAlongTangent )
//		{
//			minDistAlongTangent = distAlongTangent;
//		}
//	}
//
//	// Convert our tangent values into world points on reference edge
//	Vec2 originPoint = normal * referencePlane.distance;
//	Vec2 minPointOnReferenceEdge = originPoint + minDistAlongTangent * tangent;
//	Vec2 maxPointOnReferenceEdge = originPoint + maxDistAlongTangent * tangent;
//
//	// If the reference edge points are very close together, treat it as a single contact point
//	if ( IsNearlyEqual( minPointOnReferenceEdge, maxPointOnReferenceEdge, .0001f ) )
//	{
//		Vec2 contactPoint = minPointOnReferenceEdge - referencePlane.normal * penetrationDepth;
//		*out_contactMin = contactPoint;
//		*out_contactMax = contactPoint;
//
//		//DebugAddWorldPoint( *out_contactMin, Rgba8::GREEN, .03f );
//		//DebugAddWorldPoint( *out_contactMax, Rgba8::GREEN, .03f );
//
//		return;
//	}
//
//	// For each edge in polygon1, clip to reference edge and keep track of max and min clipped points
//	maxDistAlongTangent = -INFINITY;
//	minDistAlongTangent = INFINITY;
//	for ( int edgeIdx = 0; edgeIdx < polygonCollider1->m_polygon.GetEdgeCount(); ++edgeIdx )
//	{
//		Vec2 edgeStart;
//		Vec2 edgeEnd;
//		polygonCollider1->m_polygon.GetEdge( edgeIdx, &edgeStart, &edgeEnd );
//
//		Vec2 clippedMin;
//		Vec2 clippedMax;
//		if ( GetClippedSegmentToSegment( edgeStart, edgeEnd, minPointOnReferenceEdge, maxPointOnReferenceEdge, &clippedMin, &clippedMax ) )
//		{
//			// Check if clipped points are behind the normal, meaning they are inside polygon2 and should be considered contact points
//			// Keep track of max and min as we go so no further pruning is needed
//			if ( DotProduct2D( normal, clippedMin - originPoint ) < 0.f )
//			{
//				float distAlongTangent = DotProduct2D( clippedMin, tangent );
//				if ( distAlongTangent > maxDistAlongTangent )
//				{
//					maxDistAlongTangent = distAlongTangent;
//					*out_contactMax = clippedMin;
//				}
//				if ( distAlongTangent < minDistAlongTangent )
//				{
//					minDistAlongTangent = distAlongTangent;
//					*out_contactMin = clippedMin;
//				}
//			}
//
//			if ( DotProduct2D( normal, clippedMax - originPoint ) < 0.f )
//			{
//				float distAlongTangent = DotProduct2D( clippedMax, tangent );
//				if ( distAlongTangent > maxDistAlongTangent )
//				{
//					maxDistAlongTangent = distAlongTangent;
//					*out_contactMax = clippedMax;
//				}
//				if ( distAlongTangent < minDistAlongTangent )
//				{
//					minDistAlongTangent = distAlongTangent;
//					*out_contactMin = clippedMax;
//				}
//			}
//		}
//	}
//
//	//DebugAddWorldPoint( *out_contactMin, Rgba8::GREEN, .03f );
//	//DebugAddWorldPoint( *out_contactMax, Rgba8::GREEN, .03f );
//}
//
//
////-----------------------------------------------------------------------------------------------
//static Manifold2D PolygonVPolygonCollisionManifoldGenerator( const Collider* collider1, const Collider* collider2 )
//{
//	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
//	const PolygonCollider2D* polygonCollider1 = (const PolygonCollider2D*)collider1;
//	const PolygonCollider2D* polygonCollider2 = (const PolygonCollider2D*)collider2;
//
//	std::vector<Vec2> simplex = GetSimplexForGJKCollision( polygonCollider1, polygonCollider2 );
//	if ( simplex.size() == 0 )
//	{
//		return Manifold2D();
//	}
//
//	Vec2 edge01 = simplex[1] - simplex[0];
//	if ( DotProduct2D( edge01.GetRotated90Degrees(), simplex[2] ) < 0.f )
//	{
//		Vec2 temp = simplex[0];
//		simplex[0] = simplex[2];
//		simplex[2] = temp;
//	}
//
//	Polygon2 simplexPoly( simplex );
//	for ( int vertexIdx = 0; vertexIdx < 32; ++vertexIdx )
//	{
//		Vec2 startEdge;
//		Vec2 endEdge;
//		simplexPoly.GetClosestEdge( Vec2::ZERO, &startEdge, &endEdge );
//
//		Vec2 normal = ( endEdge - startEdge ).GetRotatedMinus90Degrees();
//		normal.Normalize();
//
//		Vec2 nextSupportPoint = GetSupportPoint( polygonCollider1, polygonCollider2, normal );
//		float distFromOriginToEdge = DotProduct2D( startEdge, normal );
//
//		if ( IsNearlyEqual( DotProduct2D( nextSupportPoint, normal ), distFromOriginToEdge, .0001f ) )
//		{
//			Manifold2D manifold;
//			manifold.normal = normal;
//			manifold.penetrationDepth = distFromOriginToEdge;
//			manifold.contactPoint1 = startEdge;
//			manifold.contactPoint2 = endEdge;
//
//			// For this next algorithm we need to use the normal from 2 to 1
//			GetContactEdgeBetweenPolygons( polygonCollider1, polygonCollider2, -normal, distFromOriginToEdge, &manifold.contactPoint1, &manifold.contactPoint2 );
//			return manifold;
//		}
//		else
//		{
//			std::vector<Vec2> newSimplex;
//			for ( int vertIdx = 0; vertIdx < (int)simplex.size(); ++vertIdx )
//			{
//				newSimplex.push_back( simplex[vertIdx] );
//				if ( simplex[vertIdx] == startEdge )
//				{
//					newSimplex.push_back( nextSupportPoint );
//				}
//			}
//
//			simplex = newSimplex;
//			simplexPoly = Polygon2( newSimplex );
//		}
//	}
//
//	return Manifold2D();
//}


////-----------------------------------------------------------------------------------------------
//// a "matrix" lookup is just a 2D array
//static CollisionManifoldGenerationCallback g_ManifoldGenerators[NUM_COLLIDER_TYPES * NUM_COLLIDER_TYPES] = {
//	/*             disc,                         polygon, */
//	/*    disc */  DiscVDiscCollisionManifoldGenerator,      nullptr,
//	/* polygon */  DiscVPolygonCollisionManifoldGenerator,   PolygonVPolygonCollisionManifoldGenerator
//};
//
////
////-----------------------------------------------------------------------------------------------
//Manifold Collider::GetCollisionManifold( const Collider* other ) const
//{
//	if ( other == nullptr
//		 || !m_rigidbody->IsEnabled()
//		 || !other->m_rigidbody->IsEnabled())
//		// || !DoAABBsOverlap2D( GetWorldBounds(), other->GetWorldBounds() ) )
//	{
//		return Manifold();
//	}
//
//	eCollider2DType myType = m_type;
//	eCollider2DType otherType = other->m_type;
//
//	if ( myType <= otherType )
//	{
//		int idx = otherType * NUM_COLLIDER_TYPES + myType;
//		CollisionManifoldGenerationCallback manifoldGenerator = g_ManifoldGenerators[idx];
//		return manifoldGenerator( this, other );
//	}
//	else
//	{
//		// flip the types when looking into the index.
//		int idx = myType * NUM_COLLIDER_TYPES + otherType;
//		CollisionManifoldGenerationCallback manifoldGenerator = g_ManifoldGenerators[idx];
//		Manifold manifold = manifoldGenerator( other, this );
//		manifold.normal *= -1.f;
//		return manifold;
//	}
//}


//-----------------------------------------------------------------------------------------------
Collider::Collider()
	: m_id( s_nextId++ )
{
}


//-----------------------------------------------------------------------------------------------
const Vec3 Collider::GetClosestPoint( const Vec2& pos ) const
{
	return GetClosestPoint( Vec3( pos, 0.f ) );
}


//-----------------------------------------------------------------------------------------------
bool Collider::Contains( const Vec2& pos ) const
{
	return Contains( Vec3( pos, 0.f ) );
}


//-----------------------------------------------------------------------------------------------
float Collider::GetBounceWith( const Collider* otherCollider ) const
{
	return m_material.m_bounciness * otherCollider->m_material.m_bounciness;
}


//-----------------------------------------------------------------------------------------------
float Collider::GetFrictionWith( const Collider* otherCollider ) const
{
	float combinedFriction = m_material.m_friction * otherCollider->m_material.m_friction;
	combinedFriction = ClampZeroToOne( combinedFriction );
	return combinedFriction;
}


////-----------------------------------------------------------------------------------------------
//Vec3 Collider::GetFarthestPointInDirection( const Vec2& direction ) const
//{
//	return GetFarthestPointInDirection( Vec3( direction, 0.f ) );
//}
