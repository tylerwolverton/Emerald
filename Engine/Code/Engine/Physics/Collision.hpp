#pragma once
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Physics/Collider.hpp"
#include "Engine/Physics/Manifold.hpp"


//-----------------------------------------------------------------------------------------------
class Collision
{
public:
	IntVec2 id = IntVec2( -1, -1 );
	uint frameNum = 0;

	Collider* myCollider = nullptr;
	Collider* theirCollider = nullptr;
	Manifold collisionManifold;
};
