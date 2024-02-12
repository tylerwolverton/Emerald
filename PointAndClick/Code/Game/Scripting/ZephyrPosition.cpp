#include "Game/Scripting/ZephyrPosition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
namespace ZephyrPositionType
{
	const char* TYPE_NAME = "Position";
}


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrPositionType::TYPE_NAME );
	metadata->RegisterMember( "x" );
	metadata->RegisterMember( "y" );
	metadata->RegisterMethod( "GetDistFromOrigin" );

	g_zephyrSubsystem->RegisterZephyrType( metadata );

	g_zephyrTypeObjFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrPosition::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase* ZephyrPosition::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrPosition* zephyrPosition = new ZephyrPosition();

	// Fill in vars from args
	if ( args != nullptr )
	{
		ZephyrTypeBase* xInit = args->GetValue( "x", (ZephyrTypeBase*)nullptr );
		ZephyrTypeBase* yInit = args->GetValue( "y", (ZephyrTypeBase*)nullptr );

		if ( xInit != nullptr && xInit->GetTypeName() == "Number" )
		{
			zephyrPosition->m_position.x = ::FromString( xInit->ToString(), zephyrPosition->m_position.x );
		}
		if ( yInit != nullptr && yInit->GetTypeName() == "Number" )
		{
			zephyrPosition->m_position.y = ::FromString( yInit->ToString(), zephyrPosition->m_position.y );
		}
	}

	// Bind methods
	zephyrPosition->BindMethod( "GetDistFromOrigin", &GetDistFromOrigin );
	zephyrPosition->BindMethod( "Set_x", &Set_x );
	zephyrPosition->BindMethod( "Set_y", &Set_y );

	return zephyrPosition;
}


//-----------------------------------------------------------------------------------------------
ZephyrPosition::ZephyrPosition()
	: ZephyrType( ZephyrPositionType::TYPE_NAME )
{
}


//-----------------------------------------------------------------------------------------------
bool ZephyrPosition::EvaluateAsBool() const
{
	return !IsNearlyEqual(m_position, Vec2::ZERO);
}

//-----------------------------------------------------------------------------------------------
ZephyrTypeBase& ZephyrPosition::operator=(ZephyrTypeBase const& other)
{
	this->m_position = ((ZephyrPosition*)&other)->m_position;
	return *this;
}


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::GetDistFromOrigin( ZephyrArgs* args )
{
	args->SetValue( "dist", m_position.GetLength() );
}


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::Set_x( ZephyrArgs* args )
{
	ZephyrTypeBase* zephyrType = args->GetValue( "value", (ZephyrTypeBase*)nullptr );
	if ( zephyrType == nullptr )
	{
		// Print error?
		return;
	}

	if ( zephyrType->GetTypeName() == "Number" )
	{
		m_position.x = ::FromString( zephyrType->ToString(), 0.f );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::Set_y( ZephyrArgs* args )
{
	ZephyrTypeBase* zephyrType = args->GetValue( "value", (ZephyrTypeBase*)nullptr );
	if ( zephyrType == nullptr )
	{
		// Print error?
		return;
	}

	if ( zephyrType->GetTypeName() == "Number" )
	{
		m_position.y = ::FromString( zephyrType->ToString(), 0.f );
	}
}
