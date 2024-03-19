#include "Engine/Zephyr/Core/ZephyrInterpreter.hpp"
#include "Engine/Zephyr/Core/ZephyrVirtualMachine.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
void ZephyrInterpreter::InterpretStateBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
													 ZephyrValueMap* globalVariables, 
													 ZephyrComponent& zephyrComponent,
													 ZephyrValueMap* stateVariables )
{
	ZephyrVirtualMachine vm( bytecodeChunk, globalVariables, zephyrComponent, nullptr, stateVariables );
	vm.InterpretBytecodeChunk();
}


//-----------------------------------------------------------------------------------------------
void ZephyrInterpreter::InterpretEventBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
													 ZephyrValueMap* globalVariables,
													 ZephyrComponent& zephyrComponent,
													 EventArgs* eventArgs, 
													 ZephyrValueMap* stateVariables )
{
	ZephyrVirtualMachine vm( bytecodeChunk, globalVariables, zephyrComponent, eventArgs, stateVariables );
	vm.InterpretBytecodeChunk();
}
