#include "Engine/Physics/CollisionResolvers/CollisionPolicies.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Math/Plane2D.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Physics/2D/DiscCollider.hpp"
#include "Engine/Physics/2D/Polygon2Collider.hpp"
#include "Engine/Renderer/DebugRender.hpp"


//-----------------------------------------------------------------------------------------------
typedef Manifold( *CollisionManifoldGenerationCallback )( const Collider*, const Collider* );


//-----------------------------------------------------------------------------------------------
static Vec2 GetSupportPoint( const Polygon2Collider* polygonCollider1, const Polygon2Collider* polygonCollider2, const Vec2& direction )
{
	return polygonCollider1->GetFarthestPointInDirection( direction ) - polygonCollider2->GetFarthestPointInDirection( -direction );
}


//-----------------------------------------------------------------------------------------------
static std::vector<Vec2> GetSimplexForGJKCollision( const Polygon2Collider* polygonCollider1, const Polygon2Collider* polygonCollider2 )
{
	// Initial point calculation
	Vec2 direction = polygonCollider2->GetWorldPosition().XY() - polygonCollider1->GetWorldPosition().XY();
	Vec2 supportPoint0 = GetSupportPoint( polygonCollider1, polygonCollider2, direction );
	Vec2 supportPoint1 = GetSupportPoint( polygonCollider1, polygonCollider2, -direction );

	Vec2 supportEdge01 = supportPoint1 - supportPoint0;
	direction = TripleProduct2D( supportEdge01, -supportPoint0, supportEdge01 );

	while ( true )
	{
		Vec2 supportPoint2 = GetSupportPoint( polygonCollider1, polygonCollider2, direction );

		// If the new support point equals an existing one, we've hit the edge of the polygon so we know there is no intersection
		if ( supportPoint2 == supportPoint0
			 || supportPoint2 == supportPoint1 )
		{
			return std::vector<Vec2>{};
		}

		// Maintain ccw order
		if ( supportEdge01.GetRotated90Degrees().GetNormalized() != direction.GetNormalized() )
		{
			Vec2 temp = supportPoint0;
			supportPoint0 = supportPoint1;
			supportPoint1 = temp;
			supportEdge01 = supportPoint1 - supportPoint0;
		}

		// Get orthogonal vectors to each of the remaining edges to test for the origin
		Vec2 supportPoint2ToOrigin = -supportPoint2;
		Vec2 supportEdge21 = supportPoint1 - supportPoint2;
		Vec2 supportEdge20 = supportPoint0 - supportPoint2;

		Vec2 orthogonal21 = TripleProduct2D( supportEdge20, supportEdge21, supportEdge21 );
		Vec2 orthogonal20 = TripleProduct2D( supportEdge21, supportEdge20, supportEdge20 );

		// If origin is on opposite side of an edge, reset support points and try again
		if ( DotProduct2D( orthogonal21, supportPoint2ToOrigin ) > 0.f )
		{
			supportPoint0 = supportPoint1;
			supportPoint1 = supportPoint2;
			direction = orthogonal21;
			continue;
		}

		if ( DotProduct2D( orthogonal20, supportPoint2ToOrigin ) > 0.f )
		{
			supportPoint1 = supportPoint0;
			supportPoint0 = supportPoint2;
			direction = orthogonal20;
			continue;
		}

		// Make sure simplex is in ccw order
		if ( orthogonal21.GetNormalized() != supportEdge21.GetRotated90Degrees().GetNormalized() )
		{
			return std::vector<Vec2>{ supportPoint0, supportPoint1, supportPoint2 };
		}

		return std::vector<Vec2>{ supportPoint2, supportPoint1, supportPoint0 };
	}

	return std::vector<Vec2>{};
}


//-----------------------------------------------------------------------------------------------
static Manifold DiscVDiscCollisionManifoldGenerator( const Collider* collider1, const Collider* collider2 )
{
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	const DiscCollider* discCollider1 = (const DiscCollider*)collider1;
	const DiscCollider* discCollider2 = (const DiscCollider*)collider2;

	Manifold manifold;

	if ( !DoAABBsOverlap2D( discCollider1->GetWorldBounds(), discCollider2->GetWorldBounds() ) )
	{
		return manifold;
	}

	manifold.normal = discCollider2->GetWorldPosition() - discCollider1->GetWorldPosition();
	manifold.normal.z = 0.f;
	manifold.normal.Normalize();

	Vec3 disc1Edge = discCollider1->GetWorldPosition() + ( manifold.normal * discCollider1->m_radius );
	Vec3 disc2Edge = discCollider2->GetWorldPosition() + ( -manifold.normal * discCollider2->m_radius );
	manifold.penetrationDepth = GetDistance2D( disc1Edge.XY(), disc2Edge.XY() );

	manifold.contactPoint1 = disc1Edge - ( manifold.normal * manifold.penetrationDepth * .5f );
	manifold.contactPoint2 = manifold.contactPoint1;

	return manifold;
}


//-----------------------------------------------------------------------------------------------
static Manifold DiscVPolygonCollisionManifoldGenerator( const Collider* collider1, const Collider* collider2 )
{
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	const DiscCollider* discCollider = (const DiscCollider*)collider1;
	const Polygon2Collider* polygonCollider = (const Polygon2Collider*)collider2;

	Vec2 closestPointOnPolygonToDisc = polygonCollider->m_polygon.GetClosestPointOnEdge( discCollider->GetWorldPosition().XY() );

	Manifold manifold;

	if ( !DoAABBsOverlap2D( discCollider->GetWorldBounds(), polygonCollider->GetWorldBounds() ) )
	{
		return manifold;
	}

	manifold.normal = Vec3( closestPointOnPolygonToDisc, 0.f ) - discCollider->GetWorldPosition();
	manifold.normal.z = 0.f;
	manifold.normal.Normalize();

	// If disc is inside polygon flip the normal to ensure it is pushed out
	if ( polygonCollider->m_polygon.Contains( discCollider->GetWorldPosition().XY() ) )
	{
		manifold.normal *= -1.f;
	}

	Vec3 closestPointOnDiscToPolygon = discCollider->GetWorldPosition() + ( manifold.normal * discCollider->m_radius );
	manifold.penetrationDepth = GetDistance2D( closestPointOnDiscToPolygon.XY(), closestPointOnPolygonToDisc );

	manifold.contactPoint1 = closestPointOnDiscToPolygon - ( manifold.normal * manifold.penetrationDepth * .5f );
	manifold.contactPoint2 = manifold.contactPoint1;

	return manifold;
}


//-----------------------------------------------------------------------------------------------
static bool GetClippedSegmentToSegment( const Vec2& segmentToClipStart, const Vec2& segmentToClipEnd,
										const Vec2& refEdgeStart, const Vec2& refEdgeEnd,
										Vec3* out_clippedMin, Vec3* out_clippedMax )
{
	Vec2 refDir = ( refEdgeEnd - refEdgeStart ).GetNormalized();

	float minDistAlongRefEdge = DotProduct2D( refEdgeStart, refDir );
	float maxDistAlongRefEdge = DotProduct2D( refEdgeEnd, refDir );

	float minDistAlongSegToClip = DotProduct2D( segmentToClipStart, refDir );
	float maxDistAlongSegToClip = DotProduct2D( segmentToClipEnd, refDir );

	float minClippedDist = Max( minDistAlongRefEdge, minDistAlongSegToClip );
	float maxClippedDist = Min( maxDistAlongRefEdge, maxDistAlongSegToClip );

	// Check for no intersection
	if ( minClippedDist > maxClippedDist )
	{
		return false;
	}

	*out_clippedMin = Vec3( RangeMapFloatVec2( minDistAlongSegToClip, maxDistAlongSegToClip, segmentToClipStart, segmentToClipEnd, minClippedDist ), 0.f );
	*out_clippedMax = Vec3( RangeMapFloatVec2( minDistAlongSegToClip, maxDistAlongSegToClip, segmentToClipStart, segmentToClipEnd, maxClippedDist ), 0.f );

	return true;
}


//-----------------------------------------------------------------------------------------------
static void GetContactEdgeBetweenPolygons( const Polygon2Collider* polygonCollider1, const Polygon2Collider* polygonCollider2,
										   const Vec2& normal, float penetrationDepth,
										   Vec3* out_contactMin, Vec3* out_contactMax )
{
	Vec2 pointOnB = polygonCollider2->GetFarthestPointInDirection( normal );
	Plane2D referencePlane( normal, pointOnB );

	// Find every point of polygon2 that lies within a tolerance of the reference plane
	const std::vector<Vec2>& pointsOfB = polygonCollider2->m_polygon.GetPoints();
	std::vector<Vec2> pointsAlongReferencePlane;
	for ( int pointIdx = 0; pointIdx < (int)pointsOfB.size(); ++pointIdx )
	{
		float distFromPlane = DotProduct2D( pointsOfB[pointIdx], normal ) - referencePlane.distance;
		if ( fabsf( distFromPlane ) < .001f )
		{
			pointsAlongReferencePlane.push_back( pointsOfB[pointIdx] );
		}
	}

	// Get the max and min values along the tangent to the normal, or the reference plane itself. 
	// This gives the bounds of a reference edge ( or segment ) to check against
	Vec2 tangent = normal.GetRotatedMinus90Degrees();
	float maxDistAlongTangent = -INFINITY;
	float minDistAlongTangent = INFINITY;
	for ( int pointIdx = 0; pointIdx < (int)pointsAlongReferencePlane.size(); ++pointIdx )
	{
		float distAlongTangent = DotProduct2D( pointsAlongReferencePlane[pointIdx], tangent );
		if ( distAlongTangent > maxDistAlongTangent )
		{
			maxDistAlongTangent = distAlongTangent;
		}
		if ( distAlongTangent < minDistAlongTangent )
		{
			minDistAlongTangent = distAlongTangent;
		}
	}

	// Convert our tangent values into world points on reference edge
	Vec2 originPoint = normal * referencePlane.distance;
	Vec2 minPointOnReferenceEdge = originPoint + minDistAlongTangent * tangent;
	Vec2 maxPointOnReferenceEdge = originPoint + maxDistAlongTangent * tangent;

	// If the reference edge points are very close together, treat it as a single contact point
	if ( IsNearlyEqual( minPointOnReferenceEdge, maxPointOnReferenceEdge, .0001f ) )
	{
		Vec2 contactPoint = minPointOnReferenceEdge - referencePlane.normal * penetrationDepth;
		*out_contactMin = Vec3( contactPoint, 0.f );
		*out_contactMax = Vec3( contactPoint, 0.f );

		//DebugAddWorldPoint( *out_contactMin, Rgba8::GREEN, .03f );
		//DebugAddWorldPoint( *out_contactMax, Rgba8::GREEN, .03f );

		return;
	}

	// For each edge in polygon1, clip to reference edge and keep track of max and min clipped points
	maxDistAlongTangent = -INFINITY;
	minDistAlongTangent = INFINITY;
	for ( int edgeIdx = 0; edgeIdx < polygonCollider1->m_polygon.GetEdgeCount(); ++edgeIdx )
	{
		Vec2 edgeStart;
		Vec2 edgeEnd;
		polygonCollider1->m_polygon.GetEdge( edgeIdx, &edgeStart, &edgeEnd );

		Vec3 clippedMin;
		Vec3 clippedMax;
		if ( GetClippedSegmentToSegment( edgeStart, edgeEnd, minPointOnReferenceEdge, maxPointOnReferenceEdge, &clippedMin, &clippedMax ) )
		{
			// Check if clipped points are behind the normal, meaning they are inside polygon2 and should be considered contact points
			// Keep track of max and min as we go so no further pruning is needed
			if ( DotProduct2D( normal, clippedMin.XY() - originPoint ) < 0.f )
			{
				float distAlongTangent = DotProduct2D( clippedMin.XY(), tangent );
				if ( distAlongTangent > maxDistAlongTangent )
				{
					maxDistAlongTangent = distAlongTangent;
					*out_contactMax = clippedMin;
				}
				if ( distAlongTangent < minDistAlongTangent )
				{
					minDistAlongTangent = distAlongTangent;
					*out_contactMin = clippedMin;
				}
			}

			if ( DotProduct2D( normal, clippedMax.XY() - originPoint ) < 0.f )
			{
				float distAlongTangent = DotProduct2D( clippedMax.XY(), tangent );
				if ( distAlongTangent > maxDistAlongTangent )
				{
					maxDistAlongTangent = distAlongTangent;
					*out_contactMax = clippedMax;
				}
				if ( distAlongTangent < minDistAlongTangent )
				{
					minDistAlongTangent = distAlongTangent;
					*out_contactMin = clippedMax;
				}
			}
		}
	}

	//DebugAddWorldPoint( *out_contactMin, Rgba8::GREEN, .03f );
	//DebugAddWorldPoint( *out_contactMax, Rgba8::GREEN, .03f );
}


//-----------------------------------------------------------------------------------------------
static Manifold PolygonVPolygonCollisionManifoldGenerator( const Collider* collider1, const Collider* collider2 )
{
	// this function is only called if the types tell me these casts are safe - so no need to a dynamic cast or type checks here.
	const Polygon2Collider* polygonCollider1 = (const Polygon2Collider*)collider1;
	const Polygon2Collider* polygonCollider2 = (const Polygon2Collider*)collider2;

	if ( !DoAABBsOverlap2D( polygonCollider1->GetWorldBounds(), polygonCollider2->GetWorldBounds() ) )
	{
		return Manifold();
	}

	std::vector<Vec2> simplex = GetSimplexForGJKCollision( polygonCollider1, polygonCollider2 );
	if ( simplex.size() == 0 )
	{
		return Manifold();
	}

	Vec2 edge01 = simplex[1] - simplex[0];
	if ( DotProduct2D( edge01.GetRotated90Degrees(), simplex[2] ) < 0.f )
	{
		Vec2 temp = simplex[0];
		simplex[0] = simplex[2];
		simplex[2] = temp;
	}

	Polygon2 simplexPoly( simplex );
	for ( int vertexIdx = 0; vertexIdx < 32; ++vertexIdx )
	{
		Vec2 startEdge;
		Vec2 endEdge;
		simplexPoly.GetClosestEdge( Vec2::ZERO, &startEdge, &endEdge );

		Vec2 normal = ( endEdge - startEdge ).GetRotatedMinus90Degrees();
		normal.Normalize();

		Vec2 nextSupportPoint = GetSupportPoint( polygonCollider1, polygonCollider2, normal );
		float distFromOriginToEdge = DotProduct2D( startEdge, normal );

		if ( IsNearlyEqual( DotProduct2D( nextSupportPoint, normal ), distFromOriginToEdge, .0001f ) )
		{
			Manifold manifold;
			manifold.normal = Vec3( normal, 0.f);
			manifold.penetrationDepth = distFromOriginToEdge;
			manifold.contactPoint1 = Vec3( startEdge, 0.f );
			manifold.contactPoint2 = Vec3( endEdge, 0.f );

			// For this next algorithm we need to use the normal from 2 to 1
			GetContactEdgeBetweenPolygons( polygonCollider1, polygonCollider2, -normal, distFromOriginToEdge, &manifold.contactPoint1, &manifold.contactPoint2 );
			return manifold;
		}
		else
		{
			std::vector<Vec2> newSimplex;
			for ( int vertIdx = 0; vertIdx < (int)simplex.size(); ++vertIdx )
			{
				newSimplex.push_back( simplex[vertIdx] );
				if ( simplex[vertIdx] == startEdge )
				{
					newSimplex.push_back( nextSupportPoint );
				}
			}

			simplex = newSimplex;
			simplexPoly = Polygon2( newSimplex );
		}
	}

	return Manifold();
}


//-----------------------------------------------------------------------------------------------
// a "matrix" lookup is just a 2D array
static CollisionManifoldGenerationCallback g_ManifoldGenerators[NUM_2D_COLLIDER_TYPES * NUM_2D_COLLIDER_TYPES] = {
	/*             disc,                         polygon, */
	/*    disc */  DiscVDiscCollisionManifoldGenerator,      nullptr,
	/* polygon */  DiscVPolygonCollisionManifoldGenerator,   PolygonVPolygonCollisionManifoldGenerator
};


//-----------------------------------------------------------------------------------------------
Manifold GJK2DCollision::GetCollisionManifoldForColliders( const Collider* collider, const Collider* otherCollider )
{
	if ( !collider->IsEnabled()
		 || !otherCollider->IsEnabled() )
	{
		return Manifold();
	}

	eColliderType myType = collider->GetType();
	eColliderType otherType = otherCollider->GetType();

	if ( myType <= otherType )
	{
		int idx = otherType * NUM_2D_COLLIDER_TYPES + myType;
		CollisionManifoldGenerationCallback manifoldGenerator = g_ManifoldGenerators[idx];
		return manifoldGenerator( collider, otherCollider );
	}
	else
	{
		// flip the types when looking into the index.
		int idx = myType * NUM_2D_COLLIDER_TYPES + otherType;
		CollisionManifoldGenerationCallback manifoldGenerator = g_ManifoldGenerators[idx];
		Manifold manifold = manifoldGenerator( otherCollider, collider );
		manifold.normal *= -1.f;
		return manifold;
	}
}


//-----------------------------------------------------------------------------------------------
float GetRotationalThingOverMomentOfInertia( Rigidbody* rigidbody, const Manifold& collisionManifold )
{
	Vec2 centerOfMassToContact = collisionManifold.GetCenterOfContactEdge().XY() - rigidbody->GetWorldPosition().XY();

	// This algorithm uses normal from b to a, so flip our normal
	float rigidbodyRotationOverInertia = DotProduct2D( centerOfMassToContact.GetRotated90Degrees(), -collisionManifold.normal.XY() );

	rigidbodyRotationOverInertia *= rigidbodyRotationOverInertia;

	rigidbodyRotationOverInertia /= rigidbody->GetMomentOfInertia();

	return rigidbodyRotationOverInertia;
}


//-----------------------------------------------------------------------------------------------
float CalculateImpulseAgainstImmoveableObject( Rigidbody* moveableRigidbody, Rigidbody* immoveableRigidbody, const Manifold& collisionManifold )
{
	float e = moveableRigidbody->GetCollider()->GetBounceWith( immoveableRigidbody->GetCollider() );

	Vec3 initialVelocity1 = immoveableRigidbody->GetImpactVelocityAtPoint( collisionManifold.GetCenterOfContactEdge() );
	Vec3 initialVelocity2 = moveableRigidbody->GetImpactVelocityAtPoint( collisionManifold.GetCenterOfContactEdge() );
	Vec2 differenceOfInitialVelocities = initialVelocity2.XY() - initialVelocity1.XY();

	float numerator = ( 1.f + e ) * DotProduct2D( differenceOfInitialVelocities, -collisionManifold.normal.XY() );

	float moveableRigidbodyRotation = GetRotationalThingOverMomentOfInertia( moveableRigidbody, collisionManifold );

	float inverseMassSum = moveableRigidbody->GetInverseMass();

	float denominator = inverseMassSum + moveableRigidbodyRotation;

	float impulseMagnitude = numerator / denominator;
	return impulseMagnitude;
}


//-----------------------------------------------------------------------------------------------
float CalculateImpulseBetweenMoveableObjects( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold )
{
	Vec3 initialVelocity1 = rigidbody1->GetImpactVelocityAtPoint( collisionManifold.GetCenterOfContactEdge() );
	Vec3 initialVelocity2 = rigidbody2->GetImpactVelocityAtPoint( collisionManifold.GetCenterOfContactEdge() );
	Vec2 differenceOfInitialVelocities = initialVelocity2.XY() - initialVelocity1.XY();

	float e = rigidbody1->GetCollider()->GetBounceWith( rigidbody2->GetCollider() );

	// This algorithm uses normal from b to a, so flip our normal
	float numerator = ( 1.f + e ) * DotProduct2D( differenceOfInitialVelocities, -collisionManifold.normal.XY() );

	float rigidbody1Rotational = GetRotationalThingOverMomentOfInertia( rigidbody1, collisionManifold );
	float rigidbody2Rotational = GetRotationalThingOverMomentOfInertia( rigidbody2, collisionManifold );

	float inverseMassSum = rigidbody1->GetInverseMass() + rigidbody2->GetInverseMass();

	float denominator = inverseMassSum + rigidbody1Rotational + rigidbody2Rotational;

	float impulseMagnitude = numerator / denominator;
	return impulseMagnitude;

}


//-----------------------------------------------------------------------------------------------
void GJK2DCollision::ApplyCollisionImpulses( Rigidbody* rigidbody1, Rigidbody* rigidbody2, const Manifold& collisionManifold )
{
	Manifold tangentManifold = collisionManifold;
	tangentManifold.normal = Vec3( collisionManifold.normal.XY().GetRotatedMinus90Degrees(), 0.f );

	//DebugAddWorldArrow( collisionManifold.GetCenterOfContactEdge(), collisionManifold.GetCenterOfContactEdge() + collisionManifold.normal, Rgba8::BLUE );
	//DebugAddWorldArrow( tangentManifold.GetCenterOfContactEdge(), tangentManifold.GetCenterOfContactEdge() + tangentManifold.normal, Rgba8::GREEN );

	// Handle dynamic vs dynamic
	if ( rigidbody1->GetSimulationMode() == SIMULATION_MODE_DYNAMIC
		 && rigidbody2->GetSimulationMode() == SIMULATION_MODE_DYNAMIC )
	{
		float normalImpulseMagnitude = CalculateImpulseBetweenMoveableObjects( rigidbody1, rigidbody2, collisionManifold );
		Vec2 impulsePosition1 = GetNearestPointOnLineSegment2D( rigidbody1->GetCenterOfMass().XY(), collisionManifold.contactPoint1.XY(), collisionManifold.contactPoint2.XY() );
		Vec2 impulsePosition2 = GetNearestPointOnLineSegment2D( rigidbody2->GetCenterOfMass().XY(), collisionManifold.contactPoint1.XY(), collisionManifold.contactPoint2.XY() );

		rigidbody1->ApplyImpulseAt( normalImpulseMagnitude * -collisionManifold.normal, Vec3( impulsePosition1, 0.f ) );
		rigidbody2->ApplyImpulseAt( normalImpulseMagnitude * collisionManifold.normal, Vec3( impulsePosition2, 0.f ) );

		float tangentImpulseMagnitude = CalculateImpulseBetweenMoveableObjects( rigidbody1, rigidbody2, tangentManifold );
		if ( fabsf( tangentImpulseMagnitude ) > fabsf( normalImpulseMagnitude ) )
		{
			tangentImpulseMagnitude = SignFloat( tangentImpulseMagnitude ) * fabsf( normalImpulseMagnitude );
		}

		rigidbody1->ApplyImpulseAt( tangentImpulseMagnitude * -tangentManifold.normal, tangentManifold.GetCenterOfContactEdge() );
		rigidbody2->ApplyImpulseAt( tangentImpulseMagnitude * tangentManifold.normal, tangentManifold.GetCenterOfContactEdge() );

		return;
	}

	// Handle other valid simulation mode combinations
	Rigidbody* immoveableObj = nullptr;
	Rigidbody* moveableObj = nullptr;
	Manifold normalManifold = collisionManifold;
	if ( ( rigidbody1->GetSimulationMode() == SIMULATION_MODE_STATIC || rigidbody1->GetSimulationMode() == SIMULATION_MODE_KINEMATIC )
		 && rigidbody2->GetSimulationMode() == SIMULATION_MODE_DYNAMIC )
	{
		immoveableObj = rigidbody1;
		moveableObj = rigidbody2;
	}
	else if ( ( rigidbody2->GetSimulationMode() == SIMULATION_MODE_STATIC || rigidbody2->GetSimulationMode() == SIMULATION_MODE_KINEMATIC )
			  && rigidbody1->GetSimulationMode() == SIMULATION_MODE_DYNAMIC )
	{
		immoveableObj = rigidbody2;
		moveableObj = rigidbody1;

		normalManifold.normal *= -1.f;
		tangentManifold.normal = Vec3( collisionManifold.normal.XY().GetRotatedMinus90Degrees(), 0.f );
	}
	else
	{
		// Don't apply impulse in this situation
		return;
	}

	float impulseMagnitude = CalculateImpulseAgainstImmoveableObject( moveableObj, immoveableObj, normalManifold );
	Vec2 impulsePosition = GetNearestPointOnLineSegment2D( moveableObj->GetCenterOfMass().XY(), normalManifold.contactPoint1.XY(), normalManifold.contactPoint2.XY() );

	moveableObj->ApplyImpulseAt( impulseMagnitude * normalManifold.normal, Vec3( impulsePosition, 0.f ) );

	float tangentMagnitude = CalculateImpulseAgainstImmoveableObject( moveableObj, immoveableObj, tangentManifold );
	float friction = rigidbody1->GetCollider()->GetFrictionWith( rigidbody2->GetCollider() );
	if ( fabsf( tangentMagnitude ) > fabsf( impulseMagnitude * friction ) )
	{
		tangentMagnitude = SignFloat( tangentMagnitude ) * fabsf( impulseMagnitude * friction );
	}

	moveableObj->ApplyImpulseAt( tangentMagnitude * tangentManifold.normal, tangentManifold.GetCenterOfContactEdge() );
}
