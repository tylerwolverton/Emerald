#include "Engine/Zephyr/Types/ZephyrString.hpp"


//-----------------------------------------------------------------------------------------------
const std::string ZephyrString::TYPE_NAME = "String";


//-----------------------------------------------------------------------------------------------
void ZephyrString::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrString::TYPE_NAME );

	g_zephyrSubsystem->RegisterZephyrType( metadata );

	g_zephyrTypeHandleFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrString::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrString::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrHandle zephyrStringHandle = g_zephyrSubsystem->AllocateNewZephyrTypeObject<ZephyrString>();
	ChildSmartPtr<ZephyrTypeBase, ZephyrString> zephyrStringPtr( zephyrStringHandle );

	if ( args == nullptr )
	{
		// Default constructor, don't process parameters
		return zephyrStringHandle;
	}

	// Fill in vars from args
	zephyrStringPtr->m_value = args->GetValue( "value", "" );

	return zephyrStringHandle;
}


//-----------------------------------------------------------------------------------------------
ZephyrString::ZephyrString()
	: ZephyrType( ZephyrString::TYPE_NAME )
{
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrString::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrString*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrString::SetMembersFromArgs( ZephyrArgs* args )
{
	UNUSED( args );
	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrString::SetMember( const std::string& memberName, ZephyrHandle value )
{
	UNUSED( memberName );
	UNUSED( value );
	return false;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrString::GetMember( const std::string& memberName )
{
	UNUSED( memberName );
	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrString::Equal( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::STRING )
	{
		ZephyrStringPtr otherAsStringPtr( other );
		if ( m_value == otherAsStringPtr->GetValue() )
		{
			return eZephyrComparatorResult::TRUE_VAL;
		}
		else
		{
			return eZephyrComparatorResult::FALSE_VAL;
		}
	}
	else if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::BOOL )
	{
		if ( EvaluateAsBool() == otherPtr->EvaluateAsBool() )
		{
			return eZephyrComparatorResult::TRUE_VAL;
		}
		else
		{
			return eZephyrComparatorResult::FALSE_VAL;
		}
	}

	return eZephyrComparatorResult::UNDEFINED_VAL;
}

