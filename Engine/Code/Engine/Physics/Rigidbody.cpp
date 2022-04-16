#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
Rigidbody::Rigidbody( float mass )
{
	GUARANTEE_OR_DIE( mass > 0.f, "Mass must be positive" );
	m_mass = mass;
	m_inverseMass = 1.f / m_mass;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::Update( float deltaSeconds )
{
	if ( !m_isEnabled )
	{
		return;
	}

	Vec3 oldPosition = m_worldPosition;

	Vec3 acceleration = m_forces;
	m_velocity += acceleration * deltaSeconds;
	m_worldPosition += m_velocity * deltaSeconds;

	float angularAcceleration = m_frameTorque;
	m_angularVelocity += angularAcceleration * deltaSeconds;
	m_orientationRadians += m_angularVelocity * deltaSeconds;
	
	const float twoPI = fPI * 2.f;
	while ( m_orientationRadians > twoPI )
	{
		m_orientationRadians -= twoPI;
	}
	while ( m_orientationRadians < 0.f )
	{
		m_orientationRadians += twoPI;
	}

	if ( m_collider != nullptr )
	{
		m_collider->UpdateWorldShape();
	}

	m_forces = Vec3::ZERO;
	m_frameTorque = 0.f;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::Destroy()
{
	if ( m_collider != nullptr )
	{
		m_scene->DestroyCollider( m_collider );
	}

	m_scene->DestroyRigidbody( this );
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::TakeCollider( Collider* collider )
{
	m_collider = collider;
	m_collider->m_rigidbody = this;

	m_collider->UpdateWorldShape();

	m_moment = m_collider->CalculateMoment( m_mass );
	if ( m_moment != 0.f )
	{
		m_inverseMoment = 1.f / m_moment;
	}

}


//-----------------------------------------------------------------------------------------------
void Rigidbody::SetVelocity( const Vec3& velocity )
{
	m_velocity = velocity;
}


////-----------------------------------------------------------------------------------------------
//Vec3 Rigidbody::GetImpactVelocityAtPoint( const Vec3& point )
//{
//	Vec2 contactPoint = point - m_worldPosition;
//	Vec2 tangent = contactPoint.GetRotated90Degrees();
//
//	return GetVelocity() + m_angularVelocity * tangent;
//}


//-----------------------------------------------------------------------------------------------
Vec3 Rigidbody::GetCenterOfMass() const
{
	if ( m_collider != nullptr )
	{
		return m_collider->m_worldPosition;
	}

	return m_worldPosition;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::SetPosition( const Vec3& position )
{
	m_worldPosition = position;

	if ( m_collider != nullptr )
	{
		m_collider->UpdateWorldShape();
	}
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::Translate( const Vec3& translation )
{
	m_worldPosition += translation;

	if ( m_collider != nullptr )
	{
		m_collider->UpdateWorldShape();
	}
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::RotateDegrees( float deltaDegrees )
{
	m_orientationRadians += ConvertDegreesToRadians( deltaDegrees );
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::ChangeAngularVelocity( float deltaRadians )
{
	m_angularVelocity += deltaRadians;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::SetAngularVelocity( float newAngularVelocity )
{
	m_angularVelocity = newAngularVelocity;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::ChangeMass( float deltaMass )
{
	float oldMass = m_mass;

	m_mass += deltaMass;
	m_mass = ClampMin( m_mass, .001f );

	m_inverseMass = 1.f / m_mass;

	float massRatio = m_mass / oldMass;
	m_moment *= massRatio;

	if ( m_moment != 0.f )
	{
		m_inverseMoment = 1.f / m_moment;
	}
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::ChangeDrag( float deltaDrag )
{
	m_drag += deltaDrag;

	//m_drag = ClampZeroToOne( m_drag );
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::AddForce( const Vec3& force )
{
	if ( !m_isEnabled )
	{
		return;
	}

	m_forces += force;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::AddImpulse( const Vec3& impulse )
{
	if ( !m_isEnabled )
	{
		return;
	}

	m_velocity += impulse;
}


////-----------------------------------------------------------------------------------------------
//void Rigidbody::ApplyImpulseAt( const Vec3& impulse, const Vec3& point )
//{
//	if ( !m_isEnabled )
//	{
//		return;
//	}
//
//	m_velocity += ( impulse * m_inverseMass );
//
//	Vec2 contactPoint = point - m_worldPosition;
//	contactPoint.Rotate90Degrees();
//
//	m_angularVelocity += DotProduct2D( impulse, contactPoint ) * m_inverseMoment;
//}


//-----------------------------------------------------------------------------------------------
void Rigidbody::ApplyDragForce()
{
	Vec3 dragForce = -GetVelocity() * m_drag;
	AddForce( dragForce );
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::DebugRender( RenderContext* renderer, const Rgba8& borderColor, const Rgba8& fillColor ) const
{
	//Rgba8 rigidbodyColor = m_isEnabled ? Rgba8::BLUE : Rgba8::RED;
	//Vec2 crossOffset( .1f, .1f );
	//DrawLine2D( renderer, m_worldPosition + crossOffset, m_worldPosition - crossOffset, rigidbodyColor, .03f );
	//crossOffset.x *= -1.f;
	//DrawLine2D( renderer, m_worldPosition + crossOffset, m_worldPosition - crossOffset, rigidbodyColor, .03f );

	//if ( m_collider != nullptr )
	//{
	//	m_collider->DebugRender( renderer, borderColor, fillColor );
	//}
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::Enable()
{
	if ( m_collider != nullptr )
	{
		m_collider->Enable();
	}
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::Disable()
{
	if ( m_collider != nullptr )
	{
		m_collider->Disable();
	}
}


//-----------------------------------------------------------------------------------------------
float Rigidbody::GetOrientationDegrees() const
{
	return ConvertRadiansToDegrees( m_orientationRadians );
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::SetRotationDegrees( float newRotationDegrees )
{
	m_orientationRadians = ConvertDegreesToRadians( newRotationDegrees );
}


//-----------------------------------------------------------------------------------------------
Rigidbody::~Rigidbody()
{
	//Destroy();
}
