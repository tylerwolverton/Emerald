#include "Engine/Physics/3D/SphereCollider.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Manifold.hpp"


//-----------------------------------------------------------------------------------------------
SphereCollider::SphereCollider( int id, const Vec3& localPosition, float radius )
	: m_radius( radius )
{
	m_id = id;
	m_type = COLLIDER_SPHERE;
	m_localPosition = localPosition;
}


//-----------------------------------------------------------------------------------------------
void SphereCollider::UpdateWorldShape()
{

}


//-----------------------------------------------------------------------------------------------
const Vec3 SphereCollider::GetClosestPoint( const Vec3& pos ) const
{
	return ( GetNormalizedDirectionFromAToB( m_worldPosition, pos ) * m_radius ) + m_worldPosition;
}


//-----------------------------------------------------------------------------------------------
bool SphereCollider::Contains( const Vec3& pos ) const
{
	return GetDistanceSquared3D( pos, m_worldPosition ) < ( m_radius * m_radius );
}


//-----------------------------------------------------------------------------------------------
bool SphereCollider::Intersects( const Collider* other ) const
{

}


//-----------------------------------------------------------------------------------------------
Manifold SphereCollider::GetCollisionManifold( const Collider* other ) const
{

}


//-----------------------------------------------------------------------------------------------
float SphereCollider::CalculateMoment( float mass )
{
	return .5f * mass * m_radius * m_radius * m_radius;
}


//-----------------------------------------------------------------------------------------------
Vec3 SphereCollider::GetFarthestPointInDirection( const Vec3& direction ) const
{

}


//-----------------------------------------------------------------------------------------------
void SphereCollider::DebugRender( RenderContext* renderer, const Rgba8& borderColor, const Rgba8& fillColor ) const
{

}
