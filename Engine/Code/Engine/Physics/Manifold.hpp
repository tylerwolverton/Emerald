#pragma once
#include "Engine/Math/Vec3.hpp"


//-----------------------------------------------------------------------------------------------
struct Manifold
{
public:
	Vec3 normal = Vec3::ZERO;
	Vec3 contactPoint1 = Vec3::ZERO;
	Vec3 contactPoint2 = Vec3::ZERO;
	float penetrationDepth = 0.f;

public:
	Vec3 GetCenterOfContactEdge() const { return ( contactPoint1 + contactPoint2 ) / 2.f; }
};
