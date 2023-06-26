#include "Game/Scripting/ZephyrPosition.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
IZephyrType* ZephyrPosition::ChildCloneSelf() const
{
	ZephyrPosition* zephyrPosition = new ZephyrPosition();
	zephyrPosition->m_typeName = m_typeName;

	return zephyrPosition;
}


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::CreateAndRegisterMetadata()
{
	BEGIN_REGISTER_METADATA( Position, ZephyrPosition );
	REGISTER_METADATA_MEMBER( x )
	REGISTER_METADATA_MEMBER( y )
	REGISTER_METADATA_METHOD( GetDistFromOrigin, ZephyrPosition );
	END_REGISTER_METADATA
}


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::GetDistFromOrigin( EventArgs* args )
{
	args->SetValue( "dist", m_position.GetLength() );
}
