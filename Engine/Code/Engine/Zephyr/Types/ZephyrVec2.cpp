#include "Engine/Zephyr/Types/ZephyrVec2.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
namespace ZephyrVec2Type
{
	const char* TYPE_NAME = "Vec2";
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrVec2::operator=( ZephyrTypeBase const& other )
{
	this->m_value = ( (ZephyrVec2*)&other )->m_value;
	return *this;
}


//-----------------------------------------------------------------------------------------------
void ZephyrVec2::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrVec2Type::TYPE_NAME );
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
		ZephyrTypeBase* xInit = args->GetValue( "x", (ZephyrTypeBase*)nullptr );
		ZephyrTypeBase* yInit = args->GetValue( "y", (ZephyrTypeBase*)nullptr );

		if ( xInit != nullptr && xInit->GetTypeName() == "Number" )
		{
			zephyrVec2->m_value.x = ::FromString( xInit->ToString(), zephyrVec2->m_value.x );
		}
		if ( yInit != nullptr && yInit->GetTypeName() == "Number" )
		{
			zephyrVec2->m_value.y = ::FromString( yInit->ToString(), zephyrVec2->m_value.y );
		}
	}

	// Bind methods
	zephyrVec2->BindMethod( "Set_x", &Set_x );
	zephyrVec2->BindMethod( "Set_y", &Set_y );

	return zephyrVec2;
}


//-----------------------------------------------------------------------------------------------
ZephyrVec2::ZephyrVec2()
	: ZephyrType( ZephyrVec2Type::TYPE_NAME )
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

	if ( zephyrType->GetTypeName() == "Number" )
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

	if ( zephyrType->GetTypeName() == "Number" )
	{
		m_value.y = ::FromString( zephyrType->ToString(), 0.f );
	}
}
