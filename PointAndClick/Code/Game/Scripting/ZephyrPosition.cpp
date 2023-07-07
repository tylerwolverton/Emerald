#include "Game/Scripting/ZephyrPosition.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::CreateAndRegisterMetadata()
{
	//BEGIN_REGISTER_METADATA( Position, ZephyrPosition );
	//REGISTER_METADATA_MEMBER( x )
	//REGISTER_METADATA_MEMBER( y )
	//REGISTER_METADATA_METHOD( GetDistFromOrigin, ZephyrPosition );
	//END_REGISTER_METADATA

	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( "Position" );

	metadata->memberNames.emplace_back( "x");
	metadata->memberNames.emplace_back( "y");
   
	metadata->methodNames.emplace_back( "GetDistFromOrigin" );
	//metadata.methods.emplace_back( "GetDistFromOrigin", &ZephyrPosition::GetDistFromOrigin );

	g_zephyrSubsystem->RegisterZephyrType( metadata );
	
	g_zephyrTypeObjFactory->RegisterCreator( metadata->typeName, &ZephyrPosition::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrType* ZephyrPosition::CreateAsZephyrType( ZephyrArgs* args )
{
	UNUSED( args );

	ZephyrPosition* zephyrPosition = new ZephyrPosition();

	// Fill in vars from args

	ZephyrType* newTypeObj = new ZephyrType( "Position", zephyrPosition );
	newTypeObj->RegisterMethod( "GetDistFromOrigin", std::bind( &ZephyrPosition::GetDistFromOrigin, zephyrPosition, std::placeholders::_1 ) );

	return newTypeObj;
}


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::GetDistFromOrigin( ZephyrArgs* args )
{
	//CloneSelf<ZephyrPosition>();

	args->SetValue( "dist", m_position.GetLength() );
}
