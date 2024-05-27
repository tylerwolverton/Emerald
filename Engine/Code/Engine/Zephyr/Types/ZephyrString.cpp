#include "Engine/Zephyr/Types/ZephyrString.hpp"
#include "Engine/Zephyr/Types/ZephyrBool.hpp"


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
bool ZephyrString::SetMember( const std::string& memberName, ZephyrValue& value )
{
	UNUSED( memberName );
	UNUSED( value );
	return false;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrString::GetMember( const std::string& memberName )
{
	UNUSED( memberName );
	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrString::Equal( ZephyrValue& other )
{
	std::string otherAsString;
	if ( other.TryToGetValueFrom<ZephyrString>( otherAsString ) )
	{
		if ( m_value == otherAsString )
		{
			return eZephyrComparatorResult::TRUE_VAL;
		}
		else
		{
			return eZephyrComparatorResult::FALSE_VAL;
		}
	}
	
	bool otherAsBool;
	if ( other.TryToGetValueFrom<ZephyrBool>( otherAsBool ) )
	{
		if ( EvaluateAsBool() == otherAsBool )
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

