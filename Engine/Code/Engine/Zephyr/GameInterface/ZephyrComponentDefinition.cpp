#include "Engine/Zephyr/GameInterface/ZephyrComponentDefinition.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Zephyr/Core/ZephyrScriptDefinition.hpp"


//-----------------------------------------------------------------------------------------------
void ZephyrComponentDefinition::ReloadZephyrScriptDefinition()
{
	if ( !zephyrScriptName.empty() )
	{
		zephyrScriptDef = ZephyrScriptDefinition::GetZephyrScriptDefinitionByName( zephyrScriptName );
	}
}
