#include "Engine/Zephyr/Types/ZephyrNumber.hpp"
#include "Engine/Zephyr/Types/ZephyrTypesCommon.hpp"
#include "Engine/Zephyr/Types/ZephyrVec2.hpp"
#include "Engine/Zephyr/Types/ZephyrVec3.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrNumber::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrNumber*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
void ZephyrNumber::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrEngineTypeNames::NUMBER );
	metadata->RegisterMember( "value" );

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
	: ZephyrType( ZephyrEngineTypeNames::NUMBER )
{
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrNumber::Greater( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrNumberPtr otherAsNumberPtr( other );
		if ( m_value > otherAsNumberPtr->GetValue() )
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
eZephyrComparatorResult ZephyrNumber::GreaterEqual( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrNumberPtr otherAsNumberPtr( other );
		if ( m_value >= otherAsNumberPtr->GetValue() )
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
eZephyrComparatorResult ZephyrNumber::Less( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrNumberPtr otherAsNumberPtr( other );
		if ( m_value < otherAsNumberPtr->GetValue() )
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
eZephyrComparatorResult ZephyrNumber::LessEqual( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrNumberPtr otherAsNumberPtr( other );
		if ( m_value <= otherAsNumberPtr->GetValue() )
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
eZephyrComparatorResult ZephyrNumber::Equal( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrNumberPtr otherAsNumberPtr( other );
		if ( m_value == otherAsNumberPtr->GetValue() )
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


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrNumber::Add( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrNumberPtr otherAsNumberPtr( other );
		ZephyrArgs params;
		params.SetValue("value", m_value + otherAsNumberPtr->GetValue() );
		return CreateAsZephyrType(&params);
	}

	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrNumber::Subtract( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrNumberPtr otherAsNumberPtr( other );
		ZephyrArgs params;
		params.SetValue( "value", m_value / otherAsNumberPtr->m_value );
		return CreateAsZephyrType( &params );
	}

	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrNumber::Multiply( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrNumberPtr otherAsNumberPtr( other );
		ZephyrArgs params;
		params.SetValue( "value", m_value * otherAsNumberPtr->m_value );
		return CreateAsZephyrType( &params );
	}
	else if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::VEC2 )
	{
		ZephyrVec2Ptr otherAsVec2Ptr( other );
		Vec2 newVec = otherAsVec2Ptr->GetValue() * m_value;

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		return CreateAsZephyrType( &params );
	}
	else if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::VEC3 )
	{
		ZephyrVec3Ptr otherAsVec3Ptr( other );
		Vec3 newVec = otherAsVec3Ptr->GetValue() * m_value;

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		params.SetValue( "z", newVec.z );
		return CreateAsZephyrType( &params );
	}

	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrNumber::Divide( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrNumberPtr otherAsNumberPtr( other );
		ZephyrArgs params;
		params.SetValue( "value", m_value / otherAsNumberPtr->m_value );
		return CreateAsZephyrType( &params );
	}
	else if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::VEC2 )
	{
		ZephyrVec2Ptr otherAsVec2Ptr( other );
		Vec2 newVec = otherAsVec2Ptr->GetValue() / m_value;

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		return CreateAsZephyrType( &params );
	}
	else if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::VEC3 )
	{
		ZephyrVec3Ptr otherAsVec3Ptr( other );
		Vec3 newVec = otherAsVec3Ptr->GetValue() / m_value;

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		params.SetValue( "z", newVec.z );
		return CreateAsZephyrType( &params );
	}

	return NULL_ZEPHYR_HANDLE;
}