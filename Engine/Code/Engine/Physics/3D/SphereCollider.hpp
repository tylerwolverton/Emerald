#pragma once
#include "Engine/Physics/Collider.hpp"


//-----------------------------------------------------------------------------------------------
class SphereCollider : public Collider
{
	friend struct PhysicsScene;
	friend class Rigidbody;

public:
	SphereCollider();

	float GetRadius() const													{ return m_radius; }

	virtual void UpdateWorldShape() override;

	// queries 
	virtual const Vec3 GetClosestPoint( const Vec3& pos ) const override;
	virtual bool Contains( const Vec3& pos ) const override;

	virtual float CalculateMoment( float mass ) override;

	// debug helpers
	virtual void DebugRender( const Rgba8& borderColor, const Rgba8& fillColor ) const override;

	// factory create
	static Collider* Create( ColliderParams* params );

protected:
	virtual ~SphereCollider() {}; // private - make sure this is virtual so correct deconstructor gets called

private:
	float m_radius = 1.f;
};

