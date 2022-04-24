#include "Engine/Physics/3D/SphereCollider.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Manifold.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Renderer/DebugRender.hpp"


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
	m_worldPosition = m_localPosition;

	if ( m_rigidbody != nullptr )
	{
		m_worldPosition += m_rigidbody->GetWorldPosition();
	}
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
float SphereCollider::CalculateMoment( float mass )
{
	return .5f * mass * m_radius * m_radius * m_radius;
}


//-----------------------------------------------------------------------------------------------
void SphereCollider::DebugRender( const Rgba8& borderColor, const Rgba8& fillColor ) const
{
	UNUSED( fillColor );

	DebugAddWorldWireSphere( m_worldPosition, m_radius, borderColor );
}
