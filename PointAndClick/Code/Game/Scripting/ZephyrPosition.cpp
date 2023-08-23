#include "Game/Scripting/ZephyrPosition.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
const char* TYPE_NAME = "Position";


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( TYPE_NAME );
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

	return zephyrPosition;
}


//-----------------------------------------------------------------------------------------------
ZephyrPosition::ZephyrPosition()
	: ZephyrTypeTemplate( TYPE_NAME )
{
}


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::GetDistFromOrigin( ZephyrArgs* args )
{
	args->SetValue( "dist", m_position.GetLength() );
}
