#include "Engine/Physics/3D/SphereCollider.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Manifold.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Renderer/DebugRender.hpp"


//-----------------------------------------------------------------------------------------------
SphereCollider::SphereCollider()
	: Collider()
{
	m_type = COLLIDER_SPHERE;
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


//-----------------------------------------------------------------------------------------------
Collider* SphereCollider::Create( ColliderParams* params )
{
	SphereCollider* collider = new SphereCollider();
	collider->m_localPosition = params->GetValue( "localPosition", Vec3::ZERO );
	collider->m_radius = params->GetValue( "radius", 1.f );

	return collider;
}

