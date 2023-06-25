#include "Game/Scripting/ZephyrTypeRegistrator.hpp"
#include "Game/Scripting/ZephyrPosition.hpp"


void ZephyrTypeRegistrator::RegisterZephyrTypes()
{
	ZephyrPosition::CreateAndRegisterMetadata();
}


void ZephyrTypeRegistrator::UnRegisterZephyrTypes()
{
}
