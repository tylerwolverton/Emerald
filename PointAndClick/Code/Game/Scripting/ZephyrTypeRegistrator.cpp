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
	std::map<std::string, ZephyrTypeMetadata> metaDataMap = g_zephyrSubsystem->GetRegisteredUserTypes();
	for ( auto& item : metaDataMap )
	{
		PTR_SAFE_DELETE( item.second.prototype );
	}

	metaDataMap.clear();
}
