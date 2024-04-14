#include "Engine/Zephyr/Types/ZephyrVec3.hpp"
#include "Engine/Zephyr/Types/ZephyrTypesCommon.hpp"
#include "Engine/Zephyr/Types/ZephyrNumber.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
void ZephyrVec3::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrEngineTypeNames::VEC3 );
	metadata->RegisterMember( "x", ZephyrEngineTypeNames::NUMBER );
	metadata->RegisterMember( "y", ZephyrEngineTypeNames::NUMBER );
	metadata->RegisterMember( "z", ZephyrEngineTypeNames::NUMBER );

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

	// Try to set from ZephyrVec2
	ZephyrHandle valueZephyrVec3 = args->GetValue( "value", NULL_ZEPHYR_HANDLE );
	if ( valueZephyrVec3.IsValid() && zephyrVec3Ptr->GetTypeName() == ZephyrEngineTypeNames::VEC3 )
	{
		zephyrVec3Ptr->m_value = ::FromString( zephyrVec3Ptr->ToString(), zephyrVec3Ptr->m_value );
		return zephyrVec3Handle;
	}
	
	// Try to set from ZephyrNumbers
	zephyrVec3Ptr->SetMembersFromArgs( args );

	return zephyrVec3Handle;
}


//-----------------------------------------------------------------------------------------------
ZephyrVec3::ZephyrVec3()
	: ZephyrType( ZephyrEngineTypeNames::VEC3 )
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

	ZephyrHandle zType = args->GetValue( "z", NULL_ZEPHYR_HANDLE );
	if ( zType.IsValid() )
	{
		SetMember( "z", zType );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrVec3::SetMember( const std::string& memberName, ZephyrHandle value )
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
	else if ( memberName == "z" )
	{
		if ( valuePtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
		{
			m_value.z = ::FromString( valuePtr->ToString(), m_value.z );
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrVec3::GetMember( const std::string& memberName )
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
	else if ( memberName == "z" )
	{
		params.SetValue( "value", m_value.z );
		return g_zephyrTypeHandleFactory->CreateHandle( ZephyrEngineTypeNames::NUMBER, &params );
	}

	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrVec3::Equal( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::VEC3 )
	{
		ZephyrVec3Ptr otherAsVec3Ptr( other );
		if ( m_value == otherAsVec3Ptr->GetValue() )
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
ZephyrHandle ZephyrVec3::Add( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::VEC3 )
	{
		ZephyrVec3Ptr otherAsVec3Ptr( other );
		Vec3 newVec = m_value + otherAsVec3Ptr->GetValue();

		ZephyrArgs params;
		params.SetValue( "value", newVec );
		return CreateAsZephyrType( &params );
	}

	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrVec3::Subtract( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::VEC3 )
	{
		ZephyrVec3Ptr otherAsVec3Ptr( other );
		Vec3 newVec = m_value - otherAsVec3Ptr->GetValue();

		ZephyrArgs params;
		params.SetValue( "value", newVec );
		return CreateAsZephyrType( &params );
	}

	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrVec3::Multiply( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrNumberPtr otherAsNumberPtr( other );
		Vec3 newVec = m_value * otherAsNumberPtr->GetValue();

		ZephyrArgs params;
		params.SetValue( "value", newVec );
		return CreateAsZephyrType( &params );
	}

	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrVec3::Divide( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		ZephyrNumberPtr otherAsNumberPtr( other );
		Vec3 newVec = m_value / otherAsNumberPtr->GetValue();

		ZephyrArgs params;
		params.SetValue( "value", newVec );
		return CreateAsZephyrType( &params );
	}

	return NULL_ZEPHYR_HANDLE;
}
