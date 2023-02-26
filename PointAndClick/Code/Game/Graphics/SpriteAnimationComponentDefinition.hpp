#pragma once
#include "Engine/Math/AABB2.hpp"

#include <map>
#include <string>


//-----------------------------------------------------------------------------------------------
class SpriteAnimationSetDefinition;


//-----------------------------------------------------------------------------------------------
struct SpriteAnimationComponentDefinition
{
public:
	AABB2													localDrawBounds;

	std::map< std::string, SpriteAnimationSetDefinition* >	spriteAnimSetDefs;
	SpriteAnimationSetDefinition*							defaultSpriteAnimSetDef = nullptr;

public:
	SpriteAnimationSetDefinition* GetSpriteAnimSetDef( const std::string& animSetName ) const;
};
