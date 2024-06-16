#include "Engine/Zephyr/Types/ZephyrNumber.hpp"
#include "Engine/Zephyr/Types/ZephyrBool.hpp"
#include "Engine/Zephyr/Types/ZephyrVec2.hpp"
#include "Engine/Zephyr/Types/ZephyrVec3.hpp"


//-----------------------------------------------------------------------------------------------
const std::string ZephyrNumber::TYPE_NAME = "Number";


//-----------------------------------------------------------------------------------------------
void ZephyrNumber::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrNumber::TYPE_NAME );

	g_zephyrSubsystem->RegisterZephyrType( metadata );

	g_zephyrTypeHandleFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrNumber::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrNumber::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrHandle zephyrNumberHandle = g_zephyrSubsystem->AllocateNewZephyrTypeObject<ZephyrNumber>();
	ZephyrNumberPtr zephyrNumberPtr( zephyrNumberHandle );

	if ( args == nullptr )
	{
		// Default constructor, don't process parameters
		return zephyrNumberHandle;
	}

	// Fill in vars from args
	zephyrNumberPtr->m_value = args->GetValue( "value", 0.f );

	return zephyrNumberHandle;
}


//-----------------------------------------------------------------------------------------------
ZephyrNumber::ZephyrNumber()
	: ZephyrType( ZephyrNumber::TYPE_NAME )
{
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrNumber::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrNumber*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrNumber::SetMembersFromArgs( ZephyrArgs* args )
{
	UNUSED( args );
	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrNumber::SetMember( const std::string& memberName, ZephyrValue& value )
{
	UNUSED( memberName );
	UNUSED( value );
	return false;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrNumber::GetMember( const std::string& memberName )
{
	UNUSED( memberName );
	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrNumber::Greater( ZephyrValue& other )
{
	NUMBER_TYPE otherAsNumber;
	if ( other.TryToGetValueFrom<ZephyrNumber>( otherAsNumber ) )
	{
		if ( m_value > otherAsNumber )
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
eZephyrComparatorResult ZephyrNumber::GreaterEqual( ZephyrValue& other )
{
	NUMBER_TYPE otherAsNumber;
	if ( other.TryToGetValueFrom<ZephyrNumber>( otherAsNumber ) )
	{
		if ( m_value >= otherAsNumber )
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
eZephyrComparatorResult ZephyrNumber::Less( ZephyrValue& other )
{
	NUMBER_TYPE otherAsNumber;
	if ( other.TryToGetValueFrom<ZephyrNumber>( otherAsNumber ) )
	{
		if ( m_value < otherAsNumber )
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
eZephyrComparatorResult ZephyrNumber::LessEqual( ZephyrValue& other )
{
	NUMBER_TYPE otherAsNumber;
	if ( other.TryToGetValueFrom<ZephyrNumber>( otherAsNumber ) )
	{
		if ( m_value <= otherAsNumber )
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
eZephyrComparatorResult ZephyrNumber::Equal( ZephyrValue& other )
{
	
	//if ( NUMBER_TYPE otherAsNumber; other.TryToGetValueFrom<ZephyrNumber>( otherAsNumber ) )
	//{
	//	if ( m_value == otherAsNumber )
	//	{
	//		return eZephyrComparatorResult::TRUE_VAL;
	//	}
	//	else
	//	{
	//		return eZephyrComparatorResult::FALSE_VAL;
	//	}
	//}

	//if ( other.IsType<ZephyrNumber>() )
	//{
	//	NUMBER_TYPE otherAsNumber = other.GetValueAs<ZephyrNumber, float>();

	//	if ( m_value == otherAsNumber )
	//	{
	//		return eZephyrComparatorResult::TRUE_VAL;
	//	}
	//	else
	//	{
	//		return eZephyrComparatorResult::FALSE_VAL;
	//	}
	//}

	if ( other.IsType<ZephyrNumber>() )
	{
		NUMBER_TYPE otherAsNumber = other.GetValueFrom<ZephyrNumber>( 0.f );

		if ( m_value == otherAsNumber )
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


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrNumber::Negate()
{
	return ZephyrValue( -m_value );
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrNumber::Add( ZephyrValue& other )
{
	NUMBER_TYPE otherAsNumber;
	if ( other.TryToGetValueFrom<ZephyrNumber>( otherAsNumber ) )
	{
		return ZephyrValue( m_value + otherAsNumber );
	}

	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrNumber::Subtract( ZephyrValue& other )
{
	NUMBER_TYPE otherAsNumber;
	if ( other.TryToGetValueFrom<ZephyrNumber>( otherAsNumber ) )
	{
		return ZephyrValue( m_value - otherAsNumber );
	}

	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrNumber::Multiply( ZephyrValue& other )
{
	NUMBER_TYPE otherAsNumber;
	if ( other.TryToGetValueFrom<ZephyrNumber>( otherAsNumber ) )
	{
		return ZephyrValue( m_value * otherAsNumber );
	}

	Vec2 otherAsVec2;
	if ( other.TryToGetValueFrom<ZephyrVec2>( otherAsVec2 ) )
	{
		Vec2 newVec = otherAsVec2 * m_value;

		ZephyrArgs params;
		params.SetValue( "value", newVec );

		return ZephyrValue( ZephyrVec2::TYPE_NAME, &params );
	}

	Vec3 otherAsVec3;
	if ( other.TryToGetValueFrom<ZephyrVec3>( otherAsVec3 ) )
	{
		Vec3 newVec = otherAsVec3 * m_value;

		ZephyrArgs params;
		params.SetValue( "value", newVec );

		return ZephyrValue( ZephyrVec3::TYPE_NAME, &params );
	}

	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrNumber::Divide( ZephyrValue& other )
{
	NUMBER_TYPE otherAsNumber;
	if ( other.TryToGetValueFrom<ZephyrNumber>( otherAsNumber ) )
	{
		return ZephyrValue( m_value / otherAsNumber );
	}

	return ZephyrValue::NULL_VAL;
}
