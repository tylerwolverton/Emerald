#include "Engine/Zephyr/Core/ZephyrInterpreter.hpp"
#include "Engine/Zephyr/Core/ZephyrVirtualMachine.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"


int s_numTimesCalledThisFrame = 0;


//-----------------------------------------------------------------------------------------------
void ZephyrInterpreter::BeginFrame()
{
	s_numTimesCalledThisFrame = 0;
}


//-----------------------------------------------------------------------------------------------
void ZephyrInterpreter::EndFrame()
{
	//g_devConsole->PrintString( Stringf( "NumChunksInterpreted = %i", s_numTimesCalledThisFrame ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrInterpreter::InterpretStateBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
													 ZephyrValueMap* globalVariables, 
													 ZephyrComponent& zephyrComponent,
													 ZephyrValueMap* stateVariables )
{
	++s_numTimesCalledThisFrame;
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
	++s_numTimesCalledThisFrame;
	ZephyrVirtualMachine vm( globalVariables, zephyrComponent, eventArgs, stateVariables );
	vm.InterpretBytecodeChunk( bytecodeChunk );
}
