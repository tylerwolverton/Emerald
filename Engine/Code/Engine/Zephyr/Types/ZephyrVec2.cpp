#include "Engine/Zephyr/Types/ZephyrVec2.hpp"
#include "Engine/Zephyr/Types/ZephyrBool.hpp"
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
	ZephyrValue zephyrVec2Val = args->GetValue( "value", ZephyrValue::NULL_VAL );
	if ( zephyrVec2Val.TryToGetValueFrom<ZephyrVec2>( valueVec2 ) )
	{
		zephyrVec2Ptr->m_value = valueVec2;
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

	ZephyrValue xType = args->GetValue( "x", ZephyrValue::NULL_VAL );
	if ( xType.IsValid() )
	{
		SetMember( "x", xType );
	}

	ZephyrValue yType = args->GetValue( "y", ZephyrValue::NULL_VAL );
	if ( yType.IsValid() )
	{
		SetMember( "y", yType );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrVec2::SetMember( const std::string& memberName, ZephyrValue& value )
{
	if ( !value.IsValid() )
	{
		return false;
	}

	if ( memberName == "x" )
	{
		NUMBER_TYPE x = m_value.x;
		if ( value.TryToGetValueFrom<ZephyrNumber>( x ) )
		{
			m_value.x = x;
			return true;
		}
	}
	else if ( memberName == "y" )
	{
		NUMBER_TYPE y = m_value.y;
		if ( value.TryToGetValueFrom<ZephyrNumber>( y ) )
		{
			m_value.y = y;
			return true;
		}
	}
	
	return false;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVec2::GetMember( const std::string& memberName )
{
	if ( memberName == "x" )
	{
		return ZephyrValue( m_value.x );
	}
	else if ( memberName == "y" )
	{
		return ZephyrValue( m_value.y );
	}

	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrVec2::Equal( ZephyrValue& other )
{
	Vec2 otherAsVec2;
	if ( other.TryToGetValueFrom<ZephyrVec2>( otherAsVec2 ) )
	{
		if ( m_value == otherAsVec2 )
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
ZephyrValue ZephyrVec2::Negate()
{
	ZephyrArgs args;
	args.SetValue( "value", -m_value );

	return ZephyrValue( ZephyrVec2::TYPE_NAME, &args );
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVec2::Add( ZephyrValue& other )
{
	Vec2 otherAsVec2;
	if ( other.TryToGetValueFrom<ZephyrVec2>( otherAsVec2 ) )
	{
		Vec2 newVec = m_value + otherAsVec2;

		ZephyrArgs params;
		params.SetValue( "value", newVec );

		return ZephyrValue( ZephyrVec2::TYPE_NAME, &params );
	}

	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVec2::Subtract( ZephyrValue& other )
{
	Vec2 otherAsVec2;
	if ( other.TryToGetValueFrom<ZephyrVec2>( otherAsVec2 ) )
	{
		Vec2 newVec = m_value - otherAsVec2;

		ZephyrArgs params;
		params.SetValue( "value", newVec );

		return ZephyrValue( ZephyrVec2::TYPE_NAME, &params );
	}

	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVec2::Multiply( ZephyrValue& other )
{
	NUMBER_TYPE otherAsNumber;
	if ( other.TryToGetValueFrom<ZephyrNumber>( otherAsNumber ) )
	{
		Vec2 newVec = m_value * otherAsNumber;

		ZephyrArgs params;
		params.SetValue( "value", newVec );

		return ZephyrValue( ZephyrVec2::TYPE_NAME, &params );
	}

	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVec2::Divide( ZephyrValue& other )
{
	NUMBER_TYPE otherAsNumber;
	if ( other.TryToGetValueFrom<ZephyrNumber>( otherAsNumber ) )
	{
		Vec2 newVec = m_value / otherAsNumber;

		ZephyrArgs params;
		params.SetValue( "value", newVec );

		return ZephyrValue( ZephyrVec2::TYPE_NAME, &params );
	}

	return ZephyrValue::NULL_VAL;
}

