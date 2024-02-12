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
eZephyrComparatorResult ZephyrVec2::Equal( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::VEC2 )
	{
		if ( m_value == ( (ZephyrVec2*)other )->m_value )
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
ZephyrTypeBase* ZephyrVec2::Add( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::VEC2 )
	{
		Vec2 newVec = m_value + ( (ZephyrVec2*)other )->m_value;

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		return CreateAsZephyrType( &params );
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrVec2::Subtract( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::VEC2 )
	{
		Vec2 newVec = m_value - ( (ZephyrVec2*)other )->m_value;

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		return CreateAsZephyrType( &params );
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrVec2::Multiply( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		Vec2 newVec = m_value * ( (ZephyrNumber*)other )->GetValue();

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		return CreateAsZephyrType( &params );
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrVec2::Divide( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		Vec2 newVec = m_value / ( (ZephyrNumber*)other )->GetValue();

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		return CreateAsZephyrType( &params );
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void ZephyrVec2::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrEngineTypeNames::VEC2 );
	metadata->RegisterMember( "x" );
	metadata->RegisterMember( "y" );

	g_zephyrSubsystem->RegisterZephyrType( metadata );

	g_zephyrTypeObjFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrVec2::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrVec2::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrVec2* zephyrVec2 = new ZephyrVec2();

	// Fill in vars from args
	if ( args != nullptr )
	{
		// Try to set from Vec2
		Vec2 valueVec2 = Vec2( -9999.f, -9999.f );
		Vec2 value = args->GetValue( "value", valueVec2 );
		if ( value != valueVec2 )
		{
			zephyrVec2->m_value = value;
		}
		else
		{
			// Try to set from ZephyrVec2
			ZephyrTypeBase* valueZephyrVec2 = args->GetValue( "value", ( ZephyrTypeBase* )nullptr );
			if ( valueZephyrVec2 != nullptr && valueZephyrVec2->GetTypeName() == ZephyrEngineTypeNames::VEC2 )
			{
				zephyrVec2->m_value = ::FromString( valueZephyrVec2->ToString(), Vec2::ZERO );
			}
			else
			{
				// Try to set from ZephyrNumbers
				ZephyrTypeBase* xInit = args->GetValue( "x", ( ZephyrTypeBase* )nullptr );
				ZephyrTypeBase* yInit = args->GetValue( "y", ( ZephyrTypeBase* )nullptr );

				if ( xInit != nullptr && xInit->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
				{
					zephyrVec2->m_value.x = ::FromString( xInit->ToString(), zephyrVec2->m_value.x );
				}
				if ( yInit != nullptr && yInit->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
				{
					zephyrVec2->m_value.y = ::FromString( yInit->ToString(), zephyrVec2->m_value.y );
				}
			}
		}
	}

	// Bind methods
	zephyrVec2->BindMethod( "Set_x", &Set_x );
	zephyrVec2->BindMethod( "Set_y", &Set_y );

	return zephyrVec2;
}


//-----------------------------------------------------------------------------------------------
ZephyrVec2::ZephyrVec2()
	: ZephyrType( ZephyrEngineTypeNames::VEC2 )
{
}


//-----------------------------------------------------------------------------------------------
void ZephyrVec2::Set_x( ZephyrArgs* args )
{
	ZephyrTypeBase* zephyrType = args->GetValue( "value", (ZephyrTypeBase*)nullptr );
	if ( zephyrType == nullptr )
	{
		// Print error?
		return;
	}

	if ( zephyrType->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		m_value.x = ::FromString( zephyrType->ToString(), 0.f );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVec2::Set_y( ZephyrArgs* args )
{
	ZephyrTypeBase* zephyrType = args->GetValue( "value", (ZephyrTypeBase*)nullptr );
	if ( zephyrType == nullptr )
	{
		// Print error?
		return;
	}

	if ( zephyrType->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		m_value.y = ::FromString( zephyrType->ToString(), 0.f );
	}
}
