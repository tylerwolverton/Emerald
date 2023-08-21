#include "Game/Scripting/ZephyrPosition.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"

#define TO_CREATE_ZEPHYR_TYPE_FUNC_NAME( className ) &className::CreateAsZephyrType
#define REGISTER_ZEPHYR_METHOD( objName, funcName, className ) objName->RegisterMethod( #funcName, std::bind( &className::funcName, objName, std::placeholders::_1 ) );
#define REGISTER_METHOD( funcName ) REGISTER_ZEPHYR_METHOD( zephyrPosition, funcName, ZephyrPosition )

//-----------------------------------------------------------------------------------------------
void ZephyrPosition::CreateAndRegisterMetadata()
{
	//BEGIN_REGISTER_METADATA( Position, ZephyrPosition );
	//REGISTER_METADATA_MEMBER( x )
	//REGISTER_METADATA_MEMBER( y )
	//REGISTER_METADATA_METHOD( GetDistFromOrigin, ZephyrPosition );
	//END_REGISTER_METADATA

	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( "Position" );
	metadata->RegisterMember( "x" );
	metadata->RegisterMember( "y" );
	metadata->RegisterMethod( "GetDistFromOrigin" );

	g_zephyrSubsystem->RegisterZephyrType( metadata );

	g_zephyrTypeObjFactory->RegisterCreator( metadata->GetTypeName(), TO_CREATE_ZEPHYR_TYPE_FUNC_NAME(ZephyrPosition) );
}


//-----------------------------------------------------------------------------------------------
ZephyrType* ZephyrPosition::CreateAsZephyrType( ZephyrArgs* args )
{
	UNUSED( args );

	ZephyrPosition* zephyrPosition = new ZephyrPosition();

	// Fill in vars from args

	//ZephyrType* newTypeObj = new ZephyrType( "Position", zephyrPosition );
	//newTypeObj->RegisterMethod( "GetDistFromOrigin", std::bind( &ZephyrPosition::GetDistFromOrigin, zephyrPosition, std::placeholders::_1 ) );
	//REGISTER_ZEPHYR_METHOD( zephyrPosition, GetDistFromOrigin, ZephyrPosition );
	REGISTER_METHOD( GetDistFromOrigin );

	return zephyrPosition;
}


//-----------------------------------------------------------------------------------------------
ZephyrPosition::ZephyrPosition()
	: ZephyrType( "Position" )
{
}


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::GetDistFromOrigin( ZephyrArgs* args )
{
	//CloneSelf<ZephyrPosition>();

	args->SetValue( "dist", m_position.GetLength() );
}
