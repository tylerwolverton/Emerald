#include "Engine/Zephyr/Types/ZephyrVec2.hpp"
#include "Engine/Zephyr/Types/ZephyrNumber.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
const std::string ZephyrVec2::TYPE_NAME = "Vec2";


//-----------------------------------------------------------------------------------------------
void ZephyrVec2::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrVec2::TYPE_NAME );
	metadata->RegisterMember( "x", ZephyrNumber::TYPE_NAME );
	metadata->RegisterMember( "y", ZephyrNumber::TYPE_NAME );

	g_zephyrSubsystem->RegisterZephyrType( metadata );

	g_zephyrTypeHandleFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrVec2::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrVec2::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrHandle zephyrVec2Handle = g_zephyrSubsystem->AllocateNewZephyrTypeObject<ZephyrVec2>();
	ChildSmartPtr<ZephyrTypeBase, ZephyrVec2> zephyrVec2Ptr( zephyrVec2Handle );
	
	if ( args == nullptr )
	{
		// Default constructor, don't process parameters
		return zephyrVec2Handle;
	}

	// Try to set from Vec2
	Vec2 valueVec2 = Vec2( -9999.f, -9999.f );
	Vec2 value = args->GetValue( "value", valueVec2 );
	if ( value != valueVec2 )
	{
		zephyrVec2Ptr->m_value = value;
		return zephyrVec2Handle;
	}

	// Try to set from ZephyrVec2
	ZephyrHandle valueZephyrVec2 = args->GetValue( "value", NULL_ZEPHYR_HANDLE );
	ZephyrSmartPtr valueZephyrVec2Ptr( valueZephyrVec2 );
	if ( valueZephyrVec2.IsValid() && valueZephyrVec2Ptr->GetTypeName() == ZephyrEngineTypeNames::VEC2 )
	{
		zephyrVec2Ptr->m_value = ::FromString( valueZephyrVec2Ptr->ToString(), zephyrVec2Ptr->m_value );
		return zephyrVec2Handle;
	}

	// Try to set from ZephyrNumbers
	zephyrVec2Ptr->SetMembersFromArgs( args );
	
	return zephyrVec2Handle;
}


//-----------------------------------------------------------------------------------------------
ZephyrVec2::ZephyrVec2()
	: ZephyrType( ZephyrVec2::TYPE_NAME )
{
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrVec2::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrVec2*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrVec2::SetMembersFromArgs( ZephyrArgs* args )
{
	if ( args == nullptr )
	{
		return true;
	}

	ZephyrHandle xType = args->GetValue( "x", NULL_ZEPHYR_HANDLE );
	if ( xType.IsValid() )
	{
		SetMember( "x", xType );
	}

	ZephyrHandle yType = args->GetValue( "y", NULL_ZEPHYR_HANDLE );
	if ( yType.IsValid() )
	{
		SetMember( "y", yType );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrVec2::SetMember( const std::string& memberName, ZephyrHandle value )
{
	if ( !value.IsValid() )
	{
		return false;
	}

	SmartPtr valuePtr( value );

	if ( memberName == "x" )
	{
		if ( valuePtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
		{
			m_value.x = ::FromString( valuePtr->ToString(), m_value.x );
			return true;
		}
	}
	else if ( memberName == "y" )
	{
		if ( valuePtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
		{
			m_value.y = ::FromString( valuePtr->ToString(), m_value.y );
			return true;
		}
	}
	
	return false;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrVec2::GetMember( const std::string& memberName )
{
	ZephyrArgs params;
	if ( memberName == "x" )
	{
		params.SetValue( "value", m_value.x );
		return g_zephyrTypeHandleFactory->CreateHandle( ZephyrEngineTypeNames::NUMBER, &params );
	}
	else if ( memberName == "y" )
	{
		params.SetValue( "value", m_value.y );
		return g_zephyrTypeHandleFactory->CreateHandle( ZephyrEngineTypeNames::NUMBER, &params );
	}

	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrVec2::Equal( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::VEC2 )
	{
		ZephyrVec2Ptr otherAsVec2Ptr( other );
		if ( m_value == otherAsVec2Ptr->GetValue() )
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
ZephyrHandle ZephyrVec2::Negate()
{
	ZephyrArgs args;
	args.SetValue( "value", -m_value );
	return g_zephyrTypeHandleFactory->CreateHandle( ZephyrEngineTypeNames::VEC2, &args );
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrVec2::Add( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::VEC2 )
	{
		ZephyrVec2Ptr otherAsVec2Ptr( other );
		Vec2 newVec = m_value + otherAsVec2Ptr->GetValue();

		ZephyrArgs params;
		params.SetValue( "value", newVec );
		return CreateAsZephyrType( &params );
	}

	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrVec2::Subtract( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::VEC2 )
	{
		ZephyrVec2Ptr otherAsVec2Ptr( other );
		Vec2 newVec = m_value - otherAsVec2Ptr->GetValue();

		ZephyrArgs params;
		params.SetValue( "value", newVec );
		return CreateAsZephyrType( &params );
	}

	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrVec2::Multiply( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrNumberPtr otherAsNumberPtr( other );
		Vec2 newVec = m_value * otherAsNumberPtr->GetValue();

		ZephyrArgs params;
		params.SetValue( "value", newVec );
		return CreateAsZephyrType( &params );
	}

	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrVec2::Divide( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrNumberPtr otherAsNumberPtr( other );
		Vec2 newVec = m_value / otherAsNumberPtr->GetValue();

		ZephyrArgs params;
		params.SetValue( "value", newVec );
		return CreateAsZephyrType( &params );
	}

	return NULL_ZEPHYR_HANDLE;
}

