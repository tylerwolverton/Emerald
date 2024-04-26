#pragma once
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class Entity;
class ZephyrBytecodeChunk;
class ZephyrComponent;
class ZephyrScriptDefinition;


//-----------------------------------------------------------------------------------------------
class ZephyrInterpreter
{
public:
	static void InterpretStateBytecodeChunk( ZephyrBytecodeChunk& bytecodeChunk,
											 ZephyrComponent& zephyrComponent );

	static void InterpretEventBytecodeChunk( ZephyrBytecodeChunk& bytecodeChunk,
											 ZephyrComponent& zephyrComponent,
											 EventArgs* eventArgs = nullptr );
};
