#pragma once
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrScriptDefinition;


//-----------------------------------------------------------------------------------------------
struct ZephyrComponentDefinition
{
public:
	bool									isScriptValid = false;
	std::string								zephyrScriptName;				// Used for reloading
	ZephyrScriptDefinition*					zephyrScriptDef = nullptr;
	ZephyrValueMap							zephyrScriptInitialValues;
	std::vector<EntityVariableInitializer>  zephyrEntityVarInits;

public:
	virtual ~ZephyrComponentDefinition() {}

	void									ReloadZephyrScriptDefinition();
};
