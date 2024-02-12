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
	: ZephyrType( ZephyrEngineTypeNames::NUMBER )
{
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrNumber::Greater( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		if ( m_value > ( (ZephyrNumber*)other )->m_value )
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
eZephyrComparatorResult ZephyrNumber::GreaterEqual( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		if ( m_value >= ( (ZephyrNumber*)other )->m_value )
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
eZephyrComparatorResult ZephyrNumber::Less( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		if ( m_value < ( (ZephyrNumber*)other )->m_value )
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
eZephyrComparatorResult ZephyrNumber::LessEqual( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		if ( m_value <= ( (ZephyrNumber*)other )->m_value )
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
eZephyrComparatorResult ZephyrNumber::Equal( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		if ( m_value == ( (ZephyrNumber*)other )->m_value )
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
ZephyrTypeBase* ZephyrNumber::Add( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrArgs params;
		params.SetValue("value", m_value + ( (ZephyrNumber*)other )->m_value );
		return CreateAsZephyrType(&params);
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrNumber::Subtract( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrArgs params;
		params.SetValue( "value", m_value / ( (ZephyrNumber*)other )->m_value );
		return CreateAsZephyrType( &params );
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrNumber::Multiply( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrArgs params;
		params.SetValue( "value", m_value * ( (ZephyrNumber*)other )->m_value );
		return CreateAsZephyrType( &params );
	}
	else if ( other->GetTypeName() == ZephyrEngineTypeNames::VEC2 )
	{
		Vec2 newVec = ( (ZephyrVec2*)other )->GetValue() * m_value;

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		return CreateAsZephyrType( &params );
	}
	else if ( other->GetTypeName() == ZephyrEngineTypeNames::VEC3 )
	{
		Vec3 newVec = ( (ZephyrVec3*)other )->GetValue() * m_value;

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		params.SetValue( "z", newVec.z );
		return CreateAsZephyrType( &params );
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrNumber::Divide( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrArgs params;
		params.SetValue( "value", m_value / ( (ZephyrNumber*)other )->m_value );
		return CreateAsZephyrType( &params );
	}
	else if ( other->GetTypeName() == ZephyrEngineTypeNames::VEC2 )
	{
		Vec2 newVec = ( (ZephyrVec2*)other )->GetValue() / m_value;

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		return CreateAsZephyrType( &params );
	}
	else if ( other->GetTypeName() == ZephyrEngineTypeNames::VEC3 )
	{
		Vec3 newVec = ( (ZephyrVec3*)other )->GetValue() / m_value;

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		params.SetValue( "z", newVec.z );
		return CreateAsZephyrType( &params );
	}

	return nullptr;
}
