#include "Engine/Zephyr/Core/ZephyrInterpreter.hpp"
#include "Engine/Zephyr/Core/ZephyrVirtualMachine.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
void ZephyrInterpreter::InterpretStateBytecodeChunk( ZephyrBytecodeChunk& bytecodeChunk, 
													 ZephyrComponent& zephyrComponent )
{
	ZephyrVirtualMachine vm( bytecodeChunk, zephyrComponent, nullptr );
	vm.InterpretBytecodeChunk();
}


//-----------------------------------------------------------------------------------------------
void ZephyrInterpreter::InterpretEventBytecodeChunk( ZephyrBytecodeChunk& bytecodeChunk, 
													 ZephyrComponent& zephyrComponent,
													 EventArgs* eventArgs )
{
	ZephyrVirtualMachine vm( bytecodeChunk, zephyrComponent, eventArgs );
	vm.InterpretBytecodeChunk();
}
