#include "Engine/Zephyr/Types/ZephyrNumber.hpp"


//-----------------------------------------------------------------------------------------------
namespace ZephyrNumberType
{
	const char* TYPE_NAME = "Number";
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrNumber::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrNumber*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
void ZephyrNumber::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrNumberType::TYPE_NAME );
	metadata->RegisterMember( "value" );

	g_zephyrSubsystem->RegisterZephyrType( metadata );

	g_zephyrTypeObjFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrNumber::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrNumber::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrNumber* zephyrNumber = new ZephyrNumber();

	if ( args == nullptr )
	{
		// Default constructor, don't process parameters
		return zephyrNumber;
	}

	// Fill in vars from args
	zephyrNumber->m_value = args->GetValue( "value", 0.f );
	//zephyrNumber->m_value = FromString( value->ToString(), 0.f );

	return zephyrNumber;
}


//-----------------------------------------------------------------------------------------------
ZephyrNumber::ZephyrNumber()
	: ZephyrType( ZephyrNumberType::TYPE_NAME )
{
}
