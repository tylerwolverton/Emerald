#include "Game/Graphics/SpriteAnimationComponent.hpp"
#include "Game/Graphics/SpriteAnimationComponentDefinition.hpp"


//-----------------------------------------------------------------------------------------------
SpriteAnimationComponent::SpriteAnimationComponent( const EntityId& parentEntityId, const SpriteAnimationComponentDefinition& spriteAnimCompDef )
	: EntityComponent( parentEntityId )
	, spriteAnimCompDef( spriteAnimCompDef )
{
	curSpriteAnimSetDef = spriteAnimCompDef.defaultSpriteAnimSetDef;
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimationComponent::ChangeSpriteAnimation( const std::string& newSpriteAnimDefSetName )
{
	SpriteAnimationSetDefinition* newSpriteAnimSetDef = spriteAnimCompDef.GetSpriteAnimSetDef( newSpriteAnimDefSetName );

	if ( newSpriteAnimSetDef == nullptr )
	{
		//g_devConsole->PrintWarning( Stringf( "Warning: Failed to change animation for entity '%s' to undefined animation '%s'", m_name.c_str(), spriteAnimDefSetName.c_str() ) );
		return;
	}

	curSpriteAnimSetDef = newSpriteAnimSetDef;
}


//-----------------------------------------------------------------------------------------------
void SpriteAnimationComponent::PlaySpriteAnimation( const std::string& newSpriteAnimDefSetName )
{
	SpriteAnimationSetDefinition* newSpriteAnimSetDef = spriteAnimCompDef.GetSpriteAnimSetDef( newSpriteAnimDefSetName );

	if ( newSpriteAnimSetDef == nullptr )
	{
		//g_devConsole->PrintWarning( Stringf( "Warning: Failed to change animation for entity '%s' to undefined animation '%s'", m_name.c_str(), spriteAnimDefSetName.c_str() ) );
		return;
	}

	curSpriteAnimSetDef = newSpriteAnimSetDef;
	cumulativeAnimSeconds = 0.f;
}
