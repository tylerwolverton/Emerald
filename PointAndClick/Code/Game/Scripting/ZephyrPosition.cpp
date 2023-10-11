#include "Game/Scripting/ZephyrPosition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
namespace ZephyrPositionType
{
	const char* TYPE_NAME = "Position";
}


//-----------------------------------------------------------------------------------------------
bool ZephyrPosition::EvaluateAsBool() const
{
	return !IsNearlyEqual( m_position, Vec2::ZERO );
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
ZephyrType* ZephyrPosition::CreateAsZephyrType( ZephyrArgs* args )
{
	UNUSED( args );

	ZephyrPosition* zephyrPosition = new ZephyrPosition();

	// Fill in vars from args


	// Bind methods
	zephyrPosition->BindMethod( "GetDistFromOrigin", &GetDistFromOrigin );
	zephyrPosition->BindMethod( "Set_x", &Set_x );
	zephyrPosition->BindMethod( "Set_y", &Set_y );

	return zephyrPosition;
}


//-----------------------------------------------------------------------------------------------
ZephyrPosition::ZephyrPosition()
	: ZephyrTypeTemplate( ZephyrPositionType::TYPE_NAME )
{
}


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::GetDistFromOrigin( ZephyrArgs* args )
{
	args->SetValue( "dist", m_position.GetLength() );
}


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::Set_x( ZephyrArgs* args )
{
	ZephyrType* zephyrType = (ZephyrType*)args->GetValue( "value", (void*)nullptr );
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
	ZephyrType* zephyrType = (ZephyrType*)args->GetValue( "value", ( void* )nullptr );
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
