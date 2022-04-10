#include "Engine/Physics/3D/Rigidbody3D.hpp"


//-----------------------------------------------------------------------------------------------
void Rigidbody3D::Update( float deltaSeconds )
{
	if ( !m_isEnabled )
	{
		return;
	}

	Vec3 oldPosition = m_worldPosition;

	Vec3 acceleration = m_sumOfForces;
	m_velocity += acceleration * deltaSeconds;
	m_worldPosition += m_velocity * deltaSeconds;
		
	//if ( m_collider != nullptr )
	//{
	//	m_collider->UpdateWorldShape();
	//}

	m_sumOfForces = Vec3::ZERO;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody3D::AddForce( const Vec3& force )
{
	m_sumOfForces += force;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody3D::AddImpulse( const Vec3& impulse )
{
	m_velocity += impulse;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody3D::ApplyDragForce()
{
	Vec3 dragForce = m_velocity * m_drag;
	AddForce( dragForce );
}


//-----------------------------------------------------------------------------------------------
Vec3 Rigidbody3D::GetWorldPosition()
{
	return m_worldPosition;
}


