#include "Game/Scripting/ZephyrTypeRegistrator.hpp"
#include "Game/Scripting/ZephyrPosition.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


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
