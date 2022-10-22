#include "Engine/Physics/Rigidbody.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/PhysicsCommon.hpp"
#include "Engine/Physics/PhysicsScene.hpp"
#include "Engine/Renderer/MeshUtils.hpp"
#include "Engine/Renderer/RenderContext.hpp"


//-----------------------------------------------------------------------------------------------
Rigidbody::Rigidbody( PhysicsScene* owningScene, const EntityId& parentEntityId )
	: m_physicsScene( owningScene )
	, m_parentEntityId( parentEntityId )
{
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::Update( float deltaSeconds )
{
	if ( !m_isEnabled )
	{
		return;
	}

	Vec3 oldPosition = m_worldPosition;

	Vec3 acceleration = m_sumOfForces;
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

	m_sumOfForces = Vec3::ZERO;
	m_frameTorque = 0.f;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::Destroy()
{
	if ( m_collider != nullptr )
	{
		m_physicsScene->DestroyCollider( m_collider );
	}

	m_physicsScene->DestroyRigidbody( this );
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
void Rigidbody::SetLayer( const std::string& layerStr )
{
	if ( !IsPhysicsLayerDefined( layerStr ) )
	{
		g_devConsole->PrintError( Stringf( "Can't set undefined physics layer '%s'", layerStr.c_str() ) );
		return;
	}

	m_layer = GetPhysicsLayerFromName( layerStr );
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
Vec3 Rigidbody::GetImpactVelocityAtPoint( const Vec3& point )
{
	Vec3 contactPoint = point - m_worldPosition;
	Vec3 tangent = Vec3( contactPoint.XY().GetRotated90Degrees(), 0.f );

	return GetVelocity() + m_angularVelocity * tangent;
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
void Rigidbody::SetMass( float mass )
{
	if ( mass <= 0.f )
	{
		g_devConsole->PrintError( "Mass must be positive" );
		return;
	}

	m_mass = mass;
	m_inverseMass = 1.f / m_mass;
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

	m_sumOfForces += force;
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


//-----------------------------------------------------------------------------------------------
void Rigidbody::ApplyImpulseAt( const Vec3& impulse, const Vec3& point )
{
	if ( !m_isEnabled )
	{
		return;
	}

	m_velocity += ( impulse * m_inverseMass );

	Vec3 contactPoint = point - m_worldPosition;
	contactPoint = Vec3( contactPoint.XY().GetRotated90Degrees(), 0.f );

	m_angularVelocity += DotProduct3D( impulse, contactPoint ) * m_inverseMoment;
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::ApplyDragForce()
{
	Vec3 dragForce = -GetVelocity() * m_drag;
	AddForce( dragForce );
}


//-----------------------------------------------------------------------------------------------
void Rigidbody::DebugRender( const Rgba8& borderColor, const Rgba8& fillColor ) const
{
	//Rgba8 rigidbodyColor = m_isEnabled ? Rgba8::BLUE : Rgba8::RED;
	
	if ( m_collider != nullptr )
	{
		m_collider->DebugRender( borderColor, fillColor );
	}
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
