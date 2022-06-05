#pragma once
#include "Engine/Physics/Collider.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Polygon2.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;


//-----------------------------------------------------------------------------------------------
class PolygonCollider2D : public Collider
{
public:
	PolygonCollider2D();

	virtual void UpdateWorldShape() override;

	// queries 
	virtual const Vec3 GetClosestPoint( const Vec3& pos ) const override;
	virtual bool Contains( const Vec3& pos ) const override;

	//virtual unsigned int CheckIfOutsideScreen( const AABB2& screenBounds, bool checkForCompletelyOffScreen ) const override;
	virtual const AABB2 GetWorldBounds() const										{ return m_polygon.m_boundingBox; }

	virtual float CalculateMoment( float mass ) override;

	Vec2 GetFarthestPointInDirection( const Vec2& direction ) const;

	// debug helpers
	virtual void DebugRender( const Rgba8& borderColor, const Rgba8& fillColor ) const override;

	// factory create
	static Collider* Create( ColliderParams* params );

protected:
	virtual ~PolygonCollider2D() {}

public:
	Polygon2 m_polygon;			// polygon that makes up body of collider
};
