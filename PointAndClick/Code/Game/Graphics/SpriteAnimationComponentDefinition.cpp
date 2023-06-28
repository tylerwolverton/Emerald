#include "Game/Graphics/SpriteAnimationComponentDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Graphics/SpriteAnimationSetDefinition.hpp"


//-----------------------------------------------------------------------------------------------
SpriteAnimationComponentDefinition::~SpriteAnimationComponentDefinition()
{
	PTR_MAP_SAFE_DELETE( spriteAnimSetDefs );
}


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
