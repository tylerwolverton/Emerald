#pragma once
//-----------------------------------------------------------------------------------------------
template<typename VecType>
struct Manifold
{
public:
	VecType normal = VecType::ZERO;
	VecType contactPoint1 = VecType::ZERO;
	VecType contactPoint2 = VecType::ZERO;
	float penetrationDepth = 0.f;

public:
	VecType GetCenterOfContactEdge() const { return ( contactPoint1 + contactPoint2 ) / 2.f; }
};
