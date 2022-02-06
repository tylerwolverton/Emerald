#pragma once
#include "Engine/Physics/Collider2D.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Polygon2.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct AABB2;


//-----------------------------------------------------------------------------------------------
class PolygonCollider2D : public Collider2D
{
public:
	PolygonCollider2D( int id, const std::vector<Vec2>& points );
	PolygonCollider2D( int id, const Polygon2& polygon );

	virtual void UpdateWorldShape() override;

	// queries 
	virtual const Vec2 GetClosestPoint( const Vec2& pos ) const override;
	virtual bool Contains( const Vec2& pos ) const override;

	virtual unsigned int CheckIfOutsideScreen( const AABB2& screenBounds, bool checkForCompletelyOffScreen ) const override;
	virtual const AABB2 GetWorldBounds() const										{ return m_polygon.m_boundingBox; }

	virtual float CalculateMoment( float mass ) override;

	virtual Vec2 GetFarthestPointInDirection( const Vec2& direction ) const override;

	// debug helpers
	virtual void DebugRender( RenderContext* renderer, const Rgba8& borderColor, const Rgba8& fillColor ) const override;

protected:
	virtual ~PolygonCollider2D();

public:
	Polygon2 m_polygon;			// polygon that makes up body of collider
};