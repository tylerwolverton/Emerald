#include "Engine/Zephyr/Types/ZephyrBool.hpp"


//-----------------------------------------------------------------------------------------------
namespace ZephyrBoolType
{
	const char* TYPE_NAME = "Bool";
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrBool::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrBool*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
void ZephyrBool::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrBoolType::TYPE_NAME );
	metadata->RegisterMember( "value" );

	g_zephyrSubsystem->RegisterZephyrType( metadata );

	g_zephyrTypeObjFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrBool::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrBool::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrBool* zephyrBool = new ZephyrBool();

	if ( args == nullptr )
	{
		// Default constructor, don't process parameters
		return zephyrBool;
	}

	// Fill in vars from args
	zephyrBool->m_value = args->GetValue( "value", false );

	return zephyrBool;
}


//-----------------------------------------------------------------------------------------------
ZephyrBool::ZephyrBool()
	: ZephyrType( ZephyrBoolType::TYPE_NAME )
{
}
