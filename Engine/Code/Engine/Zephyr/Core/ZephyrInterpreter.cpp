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
													 Entity* parentEntity,
													 ZephyrValueMap* stateVariables )
{
	++s_numTimesCalledThisFrame;
	ZephyrVirtualMachine vm;
	vm.InterpretBytecodeChunk( bytecodeChunk, globalVariables, parentEntity, nullptr, stateVariables );
}


//-----------------------------------------------------------------------------------------------
void ZephyrInterpreter::InterpretEventBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk, 
													 ZephyrValueMap* globalVariables,
													 Entity* parentEntity,
													 EventArgs* eventArgs, 
													 ZephyrValueMap* stateVariables )
{
	++s_numTimesCalledThisFrame;
	ZephyrVirtualMachine vm;
	vm.InterpretBytecodeChunk( bytecodeChunk, globalVariables, parentEntity, eventArgs, stateVariables );
}
