#include "Engine/Zephyr/Types/ZephyrBool.hpp"
#include "Engine/Zephyr/Types/ZephyrTypesCommon.hpp"


//-----------------------------------------------------------------------------------------------
void ZephyrBool::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrEngineTypeNames::BOOL );

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


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrBool::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrBool*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrBool::SetMembersFromArgs( ZephyrArgs* args )
{
	UNUSED( args );
	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrBool::SetMember( const std::string& memberName, ZephyrHandle value )
{
	UNUSED( memberName );
	UNUSED( value );
	return false;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrBool::GetMember( const std::string& memberName )
{
	UNUSED( memberName );
	return NULL_ZEPHYR_HANDLE;
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
