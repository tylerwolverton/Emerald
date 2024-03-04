#include "Engine/Zephyr/Types/ZephyrVec3.hpp"
#include "Engine/Zephyr/Types/ZephyrTypesCommon.hpp"
#include "Engine/Zephyr/Types/ZephyrNumber.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrVec3::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrVec3*)&other )->m_value;
	return *this;
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
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		params.SetValue( "z", newVec.z );
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
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		params.SetValue( "z", newVec.z );
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
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		params.SetValue( "z", newVec.z );
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
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		params.SetValue( "z", newVec.z );
		return CreateAsZephyrType( &params );
	}

	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
void ZephyrVec3::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrEngineTypeNames::VEC3 );
	metadata->RegisterMember( "x" );
	metadata->RegisterMember( "y" );
	metadata->RegisterMember( "z" );

	g_zephyrSubsystem->RegisterZephyrType( metadata );

	g_zephyrTypeHandleFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrVec3::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrVec3::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrHandle zephyrVec3Handle = g_zephyrSubsystem->AllocateNewZephyrTypeObject<ZephyrVec3>();
	ChildSmartPtr<ZephyrTypeBase, ZephyrVec3> zephyrVec3Ptr( zephyrVec3Handle );

	// Fill in vars from args
	if ( args != nullptr )
	{
		// Try to set from Vec2
		Vec3 valueVec3 = Vec3( -9999.f, -9999.f, -9999.f );
		Vec3 value = args->GetValue( "value", valueVec3 );
		if ( value != valueVec3 )
		{
			zephyrVec3Ptr->m_value = value;
		}
		else
		{
			// Try to set from ZephyrVec2
			ZephyrHandle valueZephyrVec3 = args->GetValue( "value", NULL_ZEPHYR_HANDLE );
			if ( valueZephyrVec3.IsValid() && zephyrVec3Ptr->GetTypeName() == ZephyrEngineTypeNames::VEC3 )
			{
				zephyrVec3Ptr->m_value = ::FromString( zephyrVec3Ptr->ToString(), Vec3::ZERO );
			}
			else
			{
				// Try to set from ZephyrNumbers
				ZephyrHandle xInit = args->GetValue( "x", NULL_ZEPHYR_HANDLE );
				ZephyrHandle yInit = args->GetValue( "y", NULL_ZEPHYR_HANDLE );
				ZephyrHandle zInit = args->GetValue( "z", NULL_ZEPHYR_HANDLE );
				SmartPtr xInitPtr( xInit );
				SmartPtr yInitPtr( yInit );
				SmartPtr zInitPtr( zInit );

				if ( xInit.IsValid() && xInitPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
				{
					zephyrVec3Ptr->m_value.x = ::FromString( xInitPtr->ToString(), zephyrVec3Ptr->m_value.x );
				}
				if ( yInit.IsValid() && yInitPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
				{
					zephyrVec3Ptr->m_value.y = ::FromString( yInitPtr->ToString(), zephyrVec3Ptr->m_value.y );
				}
				if ( zInit.IsValid() && zInitPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
				{
					zephyrVec3Ptr->m_value.z = ::FromString( zInitPtr->ToString(), zephyrVec3Ptr->m_value.z );
				}
			}
		}
	}

	// Bind methods
	zephyrVec3Ptr->BindMethod( "Set_x", &Set_x );
	zephyrVec3Ptr->BindMethod( "Set_y", &Set_y );
	zephyrVec3Ptr->BindMethod( "Set_z", &Set_z );

	return zephyrVec3Handle;
}


//-----------------------------------------------------------------------------------------------
ZephyrVec3::ZephyrVec3()
	: ZephyrType( ZephyrEngineTypeNames::VEC3 )
{
}


//-----------------------------------------------------------------------------------------------
void ZephyrVec3::Set_x( ZephyrArgs* args )
{
	ZephyrHandle zephyrType = args->GetValue( "value", NULL_ZEPHYR_HANDLE );
	if ( !zephyrType.IsValid() )
	{
		// Print error?
		return;
	}

	SmartPtr zephyrTypePtr( zephyrType );
	if ( zephyrTypePtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		m_value.x = ::FromString( zephyrTypePtr->ToString(), 0.f );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVec3::Set_y( ZephyrArgs* args )
{
	ZephyrHandle zephyrType = args->GetValue( "value", NULL_ZEPHYR_HANDLE );
	if ( !zephyrType.IsValid() )
	{
		// Print error?
		return;
	}

	SmartPtr zephyrTypePtr( zephyrType );
	if ( zephyrTypePtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		m_value.y = ::FromString( zephyrTypePtr->ToString(), 0.f );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVec3::Set_z( ZephyrArgs* args )
{
	ZephyrHandle zephyrType = args->GetValue( "value", NULL_ZEPHYR_HANDLE );
	if ( !zephyrType.IsValid() )
	{
		// Print error?
		return;
	}

	SmartPtr zephyrTypePtr( zephyrType );
	if ( zephyrTypePtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		m_value.z = ::FromString( zephyrTypePtr->ToString(), 0.f );
	}
}
