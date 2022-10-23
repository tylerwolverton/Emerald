#pragma once
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
class Entity;
class ZephyrComponent;
class ZephyrComponentDefinition;


//-----------------------------------------------------------------------------------------------
struct ZephyrScene
{
public:
	ZephyrComponentVector zephyrComponents;

public:
	ZephyrComponent* CreateAndAddComponent( Entity* parentEntity, const ZephyrComponentDefinition& componentDef );
	void AddComponent( ZephyrComponent* zephyrComp );
};
