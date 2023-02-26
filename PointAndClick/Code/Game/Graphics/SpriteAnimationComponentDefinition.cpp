#include "Game/Graphics/SpriteAnimationComponentDefinition.hpp"


//-----------------------------------------------------------------------------------------------
SpriteAnimationSetDefinition* SpriteAnimationComponentDefinition::GetSpriteAnimSetDef( const std::string& animSetName ) const
{
	auto mapIter = spriteAnimSetDefs.find( animSetName );

	if ( mapIter == spriteAnimSetDefs.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}
