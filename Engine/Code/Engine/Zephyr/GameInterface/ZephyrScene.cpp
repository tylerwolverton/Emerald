#include "Engine/Zephyr/GameInterface/ZephyrScene.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponent.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrScene::~ZephyrScene()
{
	Destroy();
}


//-----------------------------------------------------------------------------------------------
ZephyrComponent* ZephyrScene::CreateAndAddComponent( Entity* parentEntity, const ZephyrComponentDefinition& componentDef )
{
	ZephyrComponent* newComp = ZephyrSystem::CreateComponent( parentEntity, componentDef );

	if ( newComp != nullptr )
	{
		zephyrComponents.push_back( newComp );
	}

	return newComp;
}


//-----------------------------------------------------------------------------------------------
void ZephyrScene::Destroy()
{
	PTR_VECTOR_SAFE_DELETE( zephyrComponents );
}
