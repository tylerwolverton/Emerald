#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Physics/Manifold.hpp"


//-----------------------------------------------------------------------------------------------
template< typename ColliderType, typename VecType>
class Collision
{
public:
	IntVec2 id = IntVec2( -1, -1 );
	uint frameNum = 0;

	ColliderType* myCollider = nullptr;
	ColliderType* theirCollider = nullptr;
	Manifold<VecType> collisionManifold;
};
