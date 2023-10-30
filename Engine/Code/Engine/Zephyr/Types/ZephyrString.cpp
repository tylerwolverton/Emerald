#include "Engine/Zephyr/Types/ZephyrString.hpp"


//-----------------------------------------------------------------------------------------------
namespace ZephyrStringType
{
	const char* TYPE_NAME = "String";
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrString::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrString*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
void ZephyrString::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrStringType::TYPE_NAME );
	metadata->RegisterMember( "value" );

	g_zephyrSubsystem->RegisterZephyrType( metadata );

	g_zephyrTypeObjFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrString::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrString::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrString* zephyrString = new ZephyrString();

	if ( args == nullptr )
	{
		// Default constructor, don't process parameters
		return zephyrString;
	}

	// Fill in vars from args
	zephyrString->m_value = args->GetValue( "value", "" );

	return zephyrString;
}


//-----------------------------------------------------------------------------------------------
ZephyrString::ZephyrString()
	: ZephyrType( ZephyrStringType::TYPE_NAME )
{
}
