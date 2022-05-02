#pragma once
#include "Engine/Physics/Collider.hpp"


//-----------------------------------------------------------------------------------------------
class SphereCollider : public Collider
{
	friend class PhysicsScene;
	friend class Rigidbody;

public:
	SphereCollider( float radius, const Vec3& localPosition );

	float GetRadius() const													{ return m_radius; }

	// cache off the world shape representation of this object
	// taking into account the owning rigidbody (if no owner, local is world)
	virtual void UpdateWorldShape() override;

	// queries 
	virtual const Vec3 GetClosestPoint( const Vec3& pos ) const override;
	virtual bool Contains( const Vec3& pos ) const override;

	virtual float CalculateMoment( float mass ) override;

	// debug helpers
	virtual void DebugRender( const Rgba8& borderColor, const Rgba8& fillColor ) const override;

protected:
	virtual ~SphereCollider() {}; // private - make sure this is virtual so correct deconstructor gets called

private:
	float m_radius = 1.f;
};
