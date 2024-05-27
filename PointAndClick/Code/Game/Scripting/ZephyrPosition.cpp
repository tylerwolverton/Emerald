#include "Game/Scripting/ZephyrPosition.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
const std::string ZephyrPosition::TYPE_NAME = "Position";


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrPosition::TYPE_NAME );
	metadata->RegisterMember( "x", "" );
	metadata->RegisterMember( "y", "" );
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
				zephyrPositionPtr->SetMembersFromArgs( args );
			}
		}
	}

	// Bind methods
	zephyrPositionPtr->BindMethod( "GetDistFromOrigin", &GetDistFromOrigin );

	return zephyrPositionHandle;
}


//-----------------------------------------------------------------------------------------------
ZephyrPosition::ZephyrPosition()
	: ZephyrType( ZephyrPosition::TYPE_NAME )
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
bool ZephyrPosition::SetMembersFromArgs( ZephyrArgs* args )
{
	ZephyrHandle xType = args->GetValue( "x", NULL_ZEPHYR_HANDLE );
	if ( xType.IsValid() )
	{
		SetMember( "x", xType );
	}

	ZephyrHandle yType = args->GetValue( "y", NULL_ZEPHYR_HANDLE );
	if ( yType.IsValid() )
	{
		SetMember( "y", yType );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrPosition::SetMember( const std::string& memberName, ZephyrHandle value )
{
	if ( !value.IsValid() )
	{
		return false;
	}

	SmartPtr valuePtr( value );

	if ( memberName == "x" )
	{
		if ( valuePtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
		{
			m_position.x = ::FromString( valuePtr->ToString(), m_position.x );
			return true;
		}
	}
	else if ( memberName == "y" )
	{
		if ( valuePtr->GetTypeName() == ZephyrEngineTypeNames::NUMBER )
		{
			m_position.y = ::FromString( valuePtr->ToString(), m_position.y );
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrPosition::GetMember( const std::string& memberName )
{
	ZephyrArgs params;
	if ( memberName == "x" )
	{
		params.SetValue( "value", m_position.x );
		return g_zephyrTypeHandleFactory->CreateHandle( ZephyrEngineTypeNames::NUMBER, &params );
	}
	else if ( memberName == "y" )
	{
		params.SetValue( "value", m_position.y );
		return g_zephyrTypeHandleFactory->CreateHandle( ZephyrEngineTypeNames::NUMBER, &params );
	}

	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::GetDistFromOrigin( ZephyrArgs* args )
{
	args->SetValue( "dist", m_position.GetLength() );
}
