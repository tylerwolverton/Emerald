#include "Engine/Zephyr/Types/ZephyrBool.hpp"
#include "Engine/Zephyr/Types/ZephyrTypesCommon.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrBool::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrBool*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrBool::Equal( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( m_value == otherPtr->EvaluateAsBool() )
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

	g_zephyrTypeHandleFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrBool::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrBool::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrHandle zephyrBoolHandle = g_zephyrSubsystem->AllocateNewZephyrTypeObject<ZephyrBool>();
	ChildSmartPtr<ZephyrTypeBase, ZephyrBool> zephyrBoolPtr( zephyrBoolHandle );

	if ( args == nullptr )
	{
		// Default constructor, don't process parameters
		return zephyrBoolHandle;
	}

	// Fill in vars from args
	zephyrBoolPtr->m_value = args->GetValue( "value", false );

	return zephyrBoolHandle;
}


//-----------------------------------------------------------------------------------------------
ZephyrBool::ZephyrBool()
	: ZephyrType( ZephyrEngineTypeNames::BOOL )
{
}
