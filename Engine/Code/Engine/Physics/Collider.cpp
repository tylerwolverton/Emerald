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

