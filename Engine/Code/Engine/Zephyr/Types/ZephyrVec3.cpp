#include "Engine/Zephyr/Types/ZephyrVec3.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
namespace ZephyrVec3Type
{
	const char* TYPE_NAME = "Vec3";
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrVec3::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrVec3*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
void ZephyrVec3::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrVec3Type::TYPE_NAME );
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

		if ( xInit != nullptr && xInit->GetTypeName() == "Number" )
		{
			zephyrVec3->m_value.x = ::FromString( xInit->ToString(), zephyrVec3->m_value.x );
		}
		if ( yInit != nullptr && yInit->GetTypeName() == "Number" )
		{
			zephyrVec3->m_value.y = ::FromString( yInit->ToString(), zephyrVec3->m_value.y );
		}
		if (zInit != nullptr && zInit->GetTypeName() == "Number" )
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
	: ZephyrType( ZephyrVec3Type::TYPE_NAME )
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

	if ( zephyrType->GetTypeName() == "Number" )
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

	if ( zephyrType->GetTypeName() == "Number" )
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

	if ( zephyrType->GetTypeName() == "Number" )
	{
		m_value.z = ::FromString( zephyrType->ToString(), 0.f );
	}
}
