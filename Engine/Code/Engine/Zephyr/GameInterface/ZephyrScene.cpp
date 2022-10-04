#include "Engine/Zephyr/GameInterface/ZephyrScene.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrComponent* ZephyrScene::CreateAndAddComponent( Entity* parentEntity, const ZephyrComponentDefinition& componentDef )
{
	ZephyrComponent* newComp = ZephyrSystem::CreateComponent( parentEntity, componentDef );

	AddComponent( newComp );

	return newComp;
}


//-----------------------------------------------------------------------------------------------
void ZephyrScene::AddComponent( ZephyrComponent* zephyrComp )
{
	if ( zephyrComp != nullptr )
	{
		zephyrComponents.push_back( zephyrComp );
	}
}
