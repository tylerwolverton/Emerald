#pragma once
#include "Engine/Math/Vec3.hpp"


//-----------------------------------------------------------------------------------------------
class PhysicsSystem3D;


//-----------------------------------------------------------------------------------------------
class Rigidbody3D
{
	friend class PhysicsSystem3D;

public:
	void Update( float deltaSeconds );

	void AddForce( const Vec3& force );
	void AddImpulse( const Vec3& impulse );
	void ApplyDragForce();

	Vec3 GetWorldPosition();

private:
	PhysicsSystem3D* m_system = nullptr;
	bool m_isEnabled = true;

	float m_mass = 1.f;
	float m_inverseMass = 1.f;
	float m_drag = 0.f;

	Vec3 m_worldPosition = Vec3::ZERO;

	Vec3 m_velocity = Vec3::ZERO;
	Vec3 m_sumOfForces = Vec3::ZERO;
};
