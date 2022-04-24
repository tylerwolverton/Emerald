#pragma once
#include "Engine/Physics/Collider.hpp"
#include "Engine/Math/OBB3.hpp"


//-----------------------------------------------------------------------------------------------
class OBB3Collider : public Collider
{
	friend class PhysicsScene;
	friend class Rigidbody;

public:
	OBB3Collider( int id, const Vec3& localPosition, const OBB3& obb3 );

	OBB3 GetOBB3() const				{ return m_obb3; }
	float GetOuterRadius() const		{ return m_outerRadius; }

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
	virtual ~OBB3Collider() {}; // private - make sure this is virtual so correct deconstructor gets called

private:
	OBB3 m_obb3;
	float m_outerRadius = 1.f;
};
