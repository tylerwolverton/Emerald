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
	ZephyrVirtualMachine vm( globalVariables, zephyrComponent, nullptr, stateVariables );
	vm.InterpretBytecodeChunk( bytecodeChunk );
}


//-----------------------------------------------------------------------------------------------
void ZephyrInterpreter::InterpretEventBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
													 ZephyrValueMap* globalVariables,
													 ZephyrComponent& zephyrComponent,
													 EventArgs* eventArgs, 
													 ZephyrValueMap* stateVariables )
{
	ZephyrVirtualMachine vm( globalVariables, zephyrComponent, eventArgs, stateVariables );
	vm.InterpretBytecodeChunk( bytecodeChunk );
}
