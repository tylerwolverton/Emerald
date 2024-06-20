#include "Engine/Zephyr/Types/ZephyrVec3.hpp"
#include "Engine/Zephyr/Types/ZephyrBool.hpp"
#include "Engine/Zephyr/Types/ZephyrNumber.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
const std::string ZephyrVec3::TYPE_NAME = "Vec3";


//-----------------------------------------------------------------------------------------------
void ZephyrVec3::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrVec3::TYPE_NAME );
	metadata->RegisterMember( "x", ZephyrNumber::TYPE_NAME );
	metadata->RegisterMember( "y", ZephyrNumber::TYPE_NAME );
	metadata->RegisterMember( "z", ZephyrNumber::TYPE_NAME );

	g_zephyrSubsystem->RegisterZephyrType( metadata );

	g_zephyrTypeHandleFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrVec3::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrVec3::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrHandle zephyrVec3Handle = g_zephyrSubsystem->AllocateNewZephyrTypeObject<ZephyrVec3>();
	ChildSmartPtr<ZephyrTypeBase, ZephyrVec3> zephyrVec3Ptr( zephyrVec3Handle );

	if ( args == nullptr )
	{
		// Default constructor, don't process parameters
		return zephyrVec3Handle;
	}

	// Try to set from Vec3
	Vec3 valueVec3 = Vec3( -9999.f, -9999.f, -9999.f );
	Vec3 value = args->GetValue( "value", valueVec3 );
	if ( value != valueVec3 )
	{
		zephyrVec3Ptr->m_value = value;
		return zephyrVec3Handle;
	}

	// Try to set from ZephyrVec3
	ZephyrValue zephyrVec3Val = args->GetValue( "value", ZephyrValue::NULL_VAL );
	if ( zephyrVec3Val.TryToGetValueFrom<ZephyrVec3>( valueVec3 ) )
	{
		zephyrVec3Ptr->m_value = valueVec3;
		return zephyrVec3Handle;
	}
	
	// Try to set from ZephyrNumbers
	zephyrVec3Ptr->SetMembersFromArgs( args );

	return zephyrVec3Handle;
}


//-----------------------------------------------------------------------------------------------
ZephyrVec3::ZephyrVec3()
	: ZephyrType( ZephyrVec3::TYPE_NAME )
{
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrVec3::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrVec3*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrVec3::SetMembersFromArgs( ZephyrArgs* args )
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

	ZephyrValue zType = args->GetValue( "z", ZephyrValue::NULL_VAL );
	if ( zType.IsValid() )
	{
		SetMember( "z", zType );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrVec3::SetMember( const std::string& memberName, ZephyrValue& value )
{
	if ( !value.IsValid() )
	{
		return false;
	}

	if ( !value.IsType<ZephyrNumber>() )
	{
		return false;
	}

	if ( memberName == "x" )
	{
		m_value.x = value.GetValueFrom<ZephyrNumber>( m_value.x );
		return true;
	}
	else if ( memberName == "y" )
	{
		m_value.y = value.GetValueFrom<ZephyrNumber>( m_value.y );
		return true;
	}
	else if ( memberName == "z" )
	{
		m_value.z = value.GetValueFrom<ZephyrNumber>( m_value.z );
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVec3::GetMember( const std::string& memberName )
{
	if ( memberName == "x" )
	{
		return ZephyrValue( m_value.x );
	}
	else if ( memberName == "y" )
	{
		return ZephyrValue( m_value.y );
	}
	else if ( memberName == "z" )
	{
		return ZephyrValue( m_value.z );
	}

	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrVec3::Equal( ZephyrValue& other )
{
	Vec3 otherAsVec3;
	if ( other.TryToGetValueFrom<ZephyrVec3>( otherAsVec3 ) )
	{
		if ( m_value == otherAsVec3 )
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
ZephyrValue ZephyrVec3::Negate()
{
	ZephyrArgs args;
	args.SetValue( "value", -m_value );

	return ZephyrValue( ZephyrVec3::TYPE_NAME, &args );
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVec3::Add( ZephyrValue& other )
{
	Vec3 otherAsVec3;
	if ( other.TryToGetValueFrom<ZephyrVec3>( otherAsVec3 ) )
	{
		Vec3 newVec = m_value + otherAsVec3;

		ZephyrArgs params;
		params.SetValue( "value", newVec );

		return ZephyrValue( ZephyrVec3::TYPE_NAME, &params );
	}

	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVec3::Subtract( ZephyrValue& other )
{
	Vec3 otherAsVec3;
	if ( other.TryToGetValueFrom<ZephyrVec3>( otherAsVec3 ) )
	{
		Vec3 newVec = m_value - otherAsVec3;

		ZephyrArgs params;
		params.SetValue( "value", newVec );

		return ZephyrValue( ZephyrVec3::TYPE_NAME, &params );
	}

	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVec3::Multiply( ZephyrValue& other )
{
	NUMBER_TYPE otherAsNumber;
	if ( other.TryToGetValueFrom<ZephyrNumber>( otherAsNumber ) )
	{
		Vec3 newVec = m_value * otherAsNumber;

		ZephyrArgs params;
		params.SetValue( "value", newVec );

		return ZephyrValue( ZephyrVec3::TYPE_NAME, &params );
	}

	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVec3::Divide( ZephyrValue& other )
{
	NUMBER_TYPE otherAsNumber;
	if ( other.TryToGetValueFrom<ZephyrNumber>( otherAsNumber ) )
	{
		Vec3 newVec = m_value / otherAsNumber;

		ZephyrArgs params;
		params.SetValue( "value", newVec );

		return ZephyrValue( ZephyrVec3::TYPE_NAME, &params );
	}

	return ZephyrValue::NULL_VAL;
}
