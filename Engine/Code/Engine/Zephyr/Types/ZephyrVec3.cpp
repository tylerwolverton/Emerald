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
eZephyrComparatorResult ZephyrVec3::Equal( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::VEC3 )
	{
		if ( m_value == ( (ZephyrVec3*)other )->m_value )
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
ZephyrTypeBase* ZephyrVec3::Add( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::VEC3 )
	{
		Vec3 newVec = m_value + ( (ZephyrVec3*)other )->m_value;

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		params.SetValue( "z", newVec.z );
		return CreateAsZephyrType( &params );
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrVec3::Subtract( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::VEC3 )
	{
		Vec3 newVec = m_value - ( (ZephyrVec3*)other )->m_value;

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		params.SetValue( "z", newVec.z );
		return CreateAsZephyrType( &params );
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrVec3::Multiply( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		Vec3 newVec = m_value * ( (ZephyrNumber*)other )->GetValue();

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		params.SetValue( "z", newVec.z );
		return CreateAsZephyrType( &params );
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrVec3::Divide( ZephyrTypeBase* other )
{
	if ( other->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		Vec3 newVec = m_value / ( (ZephyrNumber*)other )->GetValue();

		ZephyrArgs params;
		params.SetValue( "x", newVec.x );
		params.SetValue( "y", newVec.y );
		params.SetValue( "z", newVec.z );
		return CreateAsZephyrType( &params );
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void ZephyrVec3::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrEngineTypeNames::VEC3 );
	metadata->RegisterMember( "x" );
	metadata->RegisterMember( "y" );
	metadata->RegisterMember( "z" );

	g_zephyrSubsystem->RegisterZephyrType( metadata );

	g_zephyrTypeObjFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrVec3::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrVec3::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrVec3* zephyrVec3 = new ZephyrVec3();

	// Fill in vars from args
	if ( args != nullptr )
	{
		ZephyrTypeBase* xInit = args->GetValue( "x", ( ZephyrTypeBase* )nullptr );
		ZephyrTypeBase* yInit = args->GetValue( "y", ( ZephyrTypeBase* )nullptr );
		ZephyrTypeBase* zInit = args->GetValue( "z", ( ZephyrTypeBase* )nullptr );

		if ( xInit != nullptr && xInit->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
		{
			zephyrVec3->m_value.x = ::FromString( xInit->ToString(), zephyrVec3->m_value.x );
		}
		if ( yInit != nullptr && yInit->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
		{
			zephyrVec3->m_value.y = ::FromString( yInit->ToString(), zephyrVec3->m_value.y );
		}
		if (zInit != nullptr && zInit->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
		{
			zephyrVec3->m_value.z = ::FromString( zInit->ToString(), zephyrVec3->m_value.z );
		}
	}

	// Bind methods
	zephyrVec3->BindMethod( "Set_x", &Set_x );
	zephyrVec3->BindMethod( "Set_y", &Set_y );
	zephyrVec3->BindMethod( "Set_z", &Set_z );

	return zephyrVec3;
}


//-----------------------------------------------------------------------------------------------
ZephyrVec3::ZephyrVec3()
	: ZephyrType( ZephyrEngineTypeNames::VEC3 )
{
}


//-----------------------------------------------------------------------------------------------
void ZephyrVec3::Set_x( ZephyrArgs* args )
{
	ZephyrTypeBase* zephyrType = args->GetValue( "value", ( ZephyrTypeBase* )nullptr );
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
void ZephyrVec3::Set_y( ZephyrArgs* args )
{
	ZephyrTypeBase* zephyrType = args->GetValue( "value", ( ZephyrTypeBase* )nullptr );
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


//-----------------------------------------------------------------------------------------------
void ZephyrVec3::Set_z( ZephyrArgs* args )
{
	ZephyrTypeBase* zephyrType = args->GetValue( "value", ( ZephyrTypeBase* )nullptr );
	if ( zephyrType == nullptr )
	{
		// Print error?
		return;
	}

	if ( zephyrType->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		m_value.z = ::FromString( zephyrType->ToString(), 0.f );
	}
}
