#include "Engine/Zephyr/Types/ZephyrString.hpp"
#include "Engine/Zephyr/Types/ZephyrTypesCommon.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrString::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrString*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrString::Equal( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::STRING )
	{
		if ( m_value == ( (ZephyrString*)other )->m_value )
		{
			return eZephyrComparatorResult::TRUE_VAL;
		}
		else
		{
			return eZephyrComparatorResult::FALSE_VAL;
		}
	}
	else if ( other->GetTypeName() == ZephyrEngineTypeNames::BOOL )
	{
		if ( EvaluateAsBool() == other->EvaluateAsBool() )
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


//-----------------------------------------------------------------------------------------------
void ZephyrString::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrEngineTypeNames::STRING );
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
	: ZephyrType( ZephyrEngineTypeNames::STRING )
{
}
