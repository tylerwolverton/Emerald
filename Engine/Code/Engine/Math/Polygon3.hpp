#pragma once
#include "Engine/Math/Vec3.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct Plane2D;
class ConvexHull2D;


//-----------------------------------------------------------------------------------------------
class Polygon3
{
public:
	Polygon3() = default;
	Polygon3( const std::vector<Vec3>& points );
	~Polygon3();

	void SetPoints( Vec3* points, int numPoints );
	void SetPoints( const std::vector<Vec3>& points );
	std::vector<Vec3> GetPoints() const { return m_points; };

	bool IsValid() const; // must have at least 3 points to be considered a polygon

	bool IsConvex() const;
	bool Contains( const Vec3& point ) const;

	// accessors
	int GetVertexCount() const;

	void Translate( const Vec3& translation );

private:
	// Stored in counter clockwise order
	std::vector<Vec3> m_points;
};
