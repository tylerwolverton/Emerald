#pragma once
#include "Engine/Physics/Collider.hpp"
#include "Engine/Math/OBB3.hpp"


//-----------------------------------------------------------------------------------------------
class OBB3Collider : public Collider
{
	friend struct PhysicsScene;
	friend class Rigidbody;

public:
	OBB3Collider();

	OBB3 GetOBB3() const				{ return m_obb3; }
	float GetOuterRadius() const		{ return m_outerRadius; }

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
	virtual ~OBB3Collider() {}; // private - make sure this is virtual so correct deconstructor gets called

private:
	OBB3 m_obb3;
	float m_outerRadius = 1.f;
};
