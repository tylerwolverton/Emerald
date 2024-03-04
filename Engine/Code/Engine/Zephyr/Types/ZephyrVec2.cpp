#include "Engine/Zephyr/Types/ZephyrVec2.hpp"
#include "Engine/Zephyr/Types/ZephyrTypesCommon.hpp"
#include "Engine/Zephyr/Types/ZephyrNumber.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrVec2::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrVec2*)&other )->m_value;
	return *this;
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
ZephyrHandle ZephyrVec2::Add( ZephyrHandle other )
{
	SmartPtr otherPtr( other );
	if ( otherPtr->GetTypeName() == ZephyrEngineTypeNames::VEC2 )
	{
		ZephyrVec2Ptr otherAsVec2Ptr( other );
		Vec2 newVec = m_value + otherAsVec2Ptr->GetValue();

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
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
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
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
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
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
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		return CreateAsZephyrType( &params );
	}

	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
void ZephyrVec2::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrEngineTypeNames::VEC2 );
	metadata->RegisterMember( "x" );
	metadata->RegisterMember( "y" );

	g_zephyrSubsystem->RegisterZephyrType( metadata );

	g_zephyrTypeHandleFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrVec2::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrVec2::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrHandle zephyrVec2Handle = g_zephyrSubsystem->AllocateNewZephyrTypeObject<ZephyrVec2>();
	ChildSmartPtr<ZephyrTypeBase, ZephyrVec2> zephyrVec2Ptr( zephyrVec2Handle );

	// Fill in vars from args
	if ( args != nullptr )
	{
		// Try to set from Vec2
		Vec2 valueVec2 = Vec2( -9999.f, -9999.f );
		Vec2 value = args->GetValue( "value", valueVec2 );
		if ( value != valueVec2 )
		{
			zephyrVec2Ptr->m_value = value;
		}
		else
		{
			// Try to set from ZephyrVec2
			ZephyrHandle valueZephyrVec2 = args->GetValue( "value", NULL_ZEPHYR_HANDLE );
			if ( valueZephyrVec2.IsValid() && zephyrVec2Ptr->GetTypeName() == ZephyrEngineTypeNames::VEC2 )
			{
				zephyrVec2Ptr->m_value = ::FromString( zephyrVec2Ptr->ToString(), Vec2::ZERO );
			}
			else
			{
				// Try to set from ZephyrNumbers
				ZephyrHandle xInit = args->GetValue( "x", NULL_ZEPHYR_HANDLE );
				ZephyrHandle yInit = args->GetValue( "y", NULL_ZEPHYR_HANDLE );
				SmartPtr xInitPtr( xInit );
				SmartPtr yInitPtr( yInit );

				if ( xInit.IsValid() && xInitPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
				{
					zephyrVec2Ptr->m_value.x = ::FromString( xInitPtr->ToString(), zephyrVec2Ptr->m_value.x );
				}
				if ( yInit.IsValid() && yInitPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
				{
					zephyrVec2Ptr->m_value.y = ::FromString( yInitPtr->ToString(), zephyrVec2Ptr->m_value.y );
				}
			}
		}
	}

	// Bind methods
	zephyrVec2Ptr->BindMethod( "Set_x", &Set_x );
	zephyrVec2Ptr->BindMethod( "Set_y", &Set_y );

	return zephyrVec2Handle;
}


//-----------------------------------------------------------------------------------------------
ZephyrVec2::ZephyrVec2()
	: ZephyrType( ZephyrEngineTypeNames::VEC2 )
{
}


//-----------------------------------------------------------------------------------------------
void ZephyrVec2::Set_x( ZephyrArgs* args )
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
void ZephyrVec2::Set_y( ZephyrArgs* args )
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
