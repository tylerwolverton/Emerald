#include "Engine/Zephyr/Types/ZephyrBool.hpp"


//-----------------------------------------------------------------------------------------------
const std::string ZephyrBool::TYPE_NAME = "Bool";


//-----------------------------------------------------------------------------------------------
void ZephyrBool::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrBool::TYPE_NAME );

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
	: ZephyrType( ZephyrBool::TYPE_NAME )
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
bool ZephyrBool::SetMember( const std::string& memberName, ZephyrValue& value )
{
	UNUSED( memberName );
	UNUSED( value );
	return false;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrBool::GetMember( const std::string& memberName )
{
	UNUSED( memberName );
	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrBool::Equal( ZephyrValue& other )
{
	if ( m_value == other.EvaluateAsBool() )
	{
		return eZephyrComparatorResult::TRUE_VAL;
	}
	else
	{
		return eZephyrComparatorResult::FALSE_VAL;
	}
}
