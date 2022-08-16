#pragma once
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <vector>

class ZephyrComponent;
class ZephyrEntity;
class ZephyrEntityDefinition;

class ZephyrSystem
{
public:
	static ZephyrComponent* CreateComponent( ZephyrEntity* parentEntity, const ZephyrEntityDefinition& entityDef );

	static void	InitializeZephyrEntityVariables();
	static void	InitializeGlobalVariables( ZephyrComponent* zephyrComp, const ZephyrValueMap& initialValues );

	static void UpdateComponents( std::vector<ZephyrComponent*>& components );
};
