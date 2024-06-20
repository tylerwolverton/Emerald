#include "Game/Scripting/ZephyrTypeRegistrator.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"
#include "Game/Scripting/ZephyrPosition.hpp"
#include "Game/Framework/GameEntity.hpp"


//-----------------------------------------------------------------------------------------------
void ZephyrTypeRegistrator::RegisterZephyrTypes()
{
	ZephyrPosition::CreateAndRegisterMetadata();
	GameEntity::AppendToMetadata();
}


//-----------------------------------------------------------------------------------------------
void ZephyrTypeRegistrator::UnRegisterZephyrTypes()
{
	g_zephyrSubsystem->ResetRegisteredUserTypes();
}
