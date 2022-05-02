#pragma once
#include "Engine/Physics/Collider.hpp"
#include "Engine/Math/Vec2.hpp"


//-----------------------------------------------------------------------------------------------
class DiscCollider : public Collider
{
public:
	DiscCollider( float radius, const Vec3& localPosition );

	virtual void UpdateWorldShape() override;

	// queries 
	virtual const Vec3 GetClosestPoint( const Vec3& pos ) const override;
	virtual bool Contains( const Vec3& pos ) const override;

	//virtual unsigned int CheckIfOutsideScreen( const AABB2& screenBounds, bool checkForCompletelyOffScreen ) const override;
	const AABB2 CalculateWorldBounds();
	AABB2 GetWorldBounds() const						{ return m_worldBounds; }

	virtual float CalculateMoment( float mass ) override;

	Vec2 GetFarthestPointInDirection( const Vec2& direction ) const;

	// debug helpers
	virtual void DebugRender( const Rgba8& borderColor, const Rgba8& fillColor ) const override;

protected:
	virtual ~DiscCollider() {}

public:
	float	m_radius = 0.f;
	AABB2	m_worldBounds;
};
