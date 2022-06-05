#include "Engine/Physics/3D/OBB3Collider.hpp"
#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Renderer/DebugRender.hpp"


//-----------------------------------------------------------------------------------------------
OBB3Collider::OBB3Collider()
	: Collider()
{
	m_type = COLLIDER_OBB3;
}


//-----------------------------------------------------------------------------------------------
void OBB3Collider::UpdateWorldShape()
{
	m_worldPosition = m_localPosition;

	if ( m_rigidbody != nullptr )
	{
		m_worldPosition += m_rigidbody->GetWorldPosition();
	}

	m_obb3.SetCenter( m_worldPosition );
}


//-----------------------------------------------------------------------------------------------
const Vec3 OBB3Collider::GetClosestPoint( const Vec3& pos ) const
{
	return m_obb3.GetNearestPoint( pos );
}


//-----------------------------------------------------------------------------------------------
bool OBB3Collider::Contains( const Vec3& pos ) const
{
	return m_obb3.IsPointInside( pos );
}


//-----------------------------------------------------------------------------------------------
float OBB3Collider::CalculateMoment( float mass )
{
	return 1.f;
}


//-----------------------------------------------------------------------------------------------
void OBB3Collider::DebugRender( const Rgba8& borderColor, const Rgba8& fillColor ) const
{
	UNUSED( fillColor );

	DebugAddWorldWireBounds( m_obb3, borderColor );
}


//-----------------------------------------------------------------------------------------------
Collider* OBB3Collider::Create( ColliderParams* params )
{
	OBB3Collider* collider = new OBB3Collider();
	collider->m_localPosition = params->GetValue( "localPosition", Vec3::ZERO );
	collider->m_obb3 = params->GetValue( "obb3", OBB3() );
	collider->m_outerRadius = collider->m_obb3.GetOuterRadius();

	return collider;
}
