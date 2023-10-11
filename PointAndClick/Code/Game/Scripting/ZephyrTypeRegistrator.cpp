#include "Game/Scripting/ZephyrTypeRegistrator.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"
#include "Game/Scripting/ZephyrPosition.hpp"


//-----------------------------------------------------------------------------------------------
void ZephyrTypeRegistrator::RegisterZephyrTypes()
{
	ZephyrPosition::CreateAndRegisterMetadata();
}


//-----------------------------------------------------------------------------------------------
void ZephyrTypeRegistrator::UnRegisterZephyrTypes()
{
	g_zephyrSubsystem->ResetRegisteredUserTypes();
}
