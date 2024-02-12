#include "Engine/Zephyr/Types/ZephyrBool.hpp"
#include "Engine/Zephyr/Types/ZephyrTypesCommon.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrBool::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrBool*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrBool::Equal( ZephyrTypeBase* other )
{
	if ( m_value == other->EvaluateAsBool() )
	{
		return eZephyrComparatorResult::TRUE_VAL;
	}
	else
	{
		return eZephyrComparatorResult::FALSE_VAL;
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrBool::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrEngineTypeNames::BOOL );
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
	: ZephyrType( ZephyrEngineTypeNames::BOOL )
{
}
