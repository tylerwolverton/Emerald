#pragma once
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <vector>


//-----------------------------------------------------------------------------------------------
struct ZephyrComponentDefinition;
class Entity;
class ZephyrComponent;


//-----------------------------------------------------------------------------------------------
struct ZephyrScene
{
public:
	ZephyrComponentVector zephyrComponents;

public:
	ZephyrComponent* CreateAndAddComponent( Entity* parentEntity, const ZephyrComponentDefinition& componentDef );
	void AddComponent( ZephyrComponent* zephyrComp );
};
