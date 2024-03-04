#include "Game/Scripting/ZephyrPosition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"
#include "Engine/Zephyr/Types/ZephyrTypesCommon.hpp"


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

	g_zephyrTypeHandleFactory->RegisterCreator( metadata->GetTypeName(), &ZephyrPosition::CreateAsZephyrType );
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrPosition::CreateAsZephyrType( ZephyrArgs* args )
{
	ZephyrHandle zephyrPositionHandle = g_zephyrSubsystem->AllocateNewZephyrTypeObject<ZephyrPosition>();
	ZephyrPositionPtr zephyrPositionPtr( zephyrPositionHandle );

	// Fill in vars from args
	if ( args != nullptr )
	{
		// Try to set from Vec2
		Vec2 valueVec2 = Vec2( -9999.f, -9999.f );
		Vec2 value = args->GetValue( "value", valueVec2 );
		if ( value != valueVec2 )
		{
			zephyrPositionPtr->m_position = value;
		}
		else
		{
			// Try to set from ZephyrVec2
			ZephyrHandle valueZephyrVec2 = args->GetValue( "value", NULL_ZEPHYR_HANDLE );
			if ( valueZephyrVec2.IsValid() && zephyrPositionPtr->GetTypeName() == ZephyrPositionType::TYPE_NAME )
			{
				zephyrPositionPtr->m_position = ::FromString( zephyrPositionPtr->ToString(), Vec2::ZERO );
			}
			else
			{
				// Try to set from ZephyrNumbers
				ZephyrHandle xInit = args->GetValue( "x", NULL_ZEPHYR_HANDLE );
				ZephyrHandle yInit = args->GetValue( "y", NULL_ZEPHYR_HANDLE );
				SmartPtr xInitPtr( xInit );
				SmartPtr yInitPtr( yInit );

				if ( xInit.IsValid() && xInitPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
				{
					zephyrPositionPtr->m_position.x = ::FromString( xInitPtr->ToString(), zephyrPositionPtr->m_position.x );
				}
				if ( yInit.IsValid() && yInitPtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
				{
					zephyrPositionPtr->m_position.y = ::FromString( yInitPtr->ToString(), zephyrPositionPtr->m_position.y );
				}
			}
		}
	}

	// Bind methods
	zephyrPositionPtr->BindMethod( "GetDistFromOrigin", &GetDistFromOrigin );
	zephyrPositionPtr->BindMethod( "Set_x", &Set_x );
	zephyrPositionPtr->BindMethod( "Set_y", &Set_y );

	return zephyrPositionHandle;
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
	ZephyrHandle zephyrType = args->GetValue( "value", NULL_ZEPHYR_HANDLE );
	if ( !zephyrType.IsValid() )
	{
		// Print error?
		return;
	}

	SmartPtr zephyrTypePtr( zephyrType );
	if ( zephyrTypePtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		m_position.x = ::FromString( zephyrTypePtr->ToString(), 0.f );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::Set_y( ZephyrArgs* args )
{
	ZephyrHandle zephyrType = args->GetValue( "value", NULL_ZEPHYR_HANDLE );
	if ( !zephyrType.IsValid() )
	{
		// Print error?
		return;
	}

	SmartPtr zephyrTypePtr( zephyrType );
	if ( zephyrTypePtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
	{
		m_position.y = ::FromString( zephyrTypePtr->ToString(), 0.f );
	}
}
