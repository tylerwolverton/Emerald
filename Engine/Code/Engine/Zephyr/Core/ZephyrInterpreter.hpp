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
	static void BeginFrame();
	static void EndFrame();

	static void InterpretStateBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk,
											 ZephyrValueMap* globalVariables,
											 ZephyrComponent& zephyrComponent,
											 ZephyrValueMap* stateVariables = nullptr );

	static void InterpretEventBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk,
											 ZephyrValueMap* globalVariables,
											 ZephyrComponent& zephyrComponent,
											 EventArgs* eventArgs = nullptr,
											 ZephyrValueMap* stateVariables = nullptr );
};
