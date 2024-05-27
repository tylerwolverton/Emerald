#include "Game/Scripting/ZephyrPosition.hpp"
#include "Engine/Zephyr/Types/ZephyrNumber.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
const std::string ZephyrPosition::TYPE_NAME = "Position";


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::CreateAndRegisterMetadata()
{
	ZephyrTypeMetadata* metadata = new ZephyrTypeMetadata( ZephyrPosition::TYPE_NAME );
	metadata->RegisterMember( "x", ZephyrNumber::TYPE_NAME );
	metadata->RegisterMember( "y", ZephyrNumber::TYPE_NAME );
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
		//else
		//{
		//	// Try to set from ZephyrVec2
		//	ZephyrHandle valueZephyrVec2 = args->GetValue( "value", NULL_ZEPHYR_HANDLE );
		//	if ( valueZephyrVec2.IsValid() && zephyrPositionPtr->GetTypeName() == ZephyrPosition::TYPE_NAME )
		//	{
		//		zephyrPositionPtr->m_position = ::FromString( zephyrPositionPtr->ToString(), Vec2::ZERO );
		//	}
		//	else
		//	{
		//		zephyrPositionPtr->SetMembersFromArgs( args );
		//	}
		//}
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
	if ( args == nullptr )
	{
		return true;
	}

	ZephyrValue xType = args->GetValue( "x", ZephyrValue::NULL_VAL );
	if ( xType.IsValid() )
	{
		SetMember( "x", xType );
	}

	ZephyrValue yType = args->GetValue( "y", ZephyrValue::NULL_VAL );
	if ( yType.IsValid() )
	{
		SetMember( "y", yType );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrPosition::SetMember( const std::string& memberName, ZephyrValue& value )
{
	if ( !value.IsValid() )
	{
		return false;
	}

	if ( memberName == "x" )
	{
		NUMBER_TYPE x = m_position.x;
		if ( value.TryToGetValueFrom<ZephyrNumber>( x ) )
		{
			m_position.x = x;
			return true;
		}
	}
	else if ( memberName == "y" )
	{
		NUMBER_TYPE y = m_position.y;
		if ( value.TryToGetValueFrom<ZephyrNumber>( y ) )
		{
			m_position.y = y;
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrPosition::GetMember( const std::string& memberName )
{
	if ( memberName == "x" )
	{
		return ZephyrValue( m_position.x );
	}
	else if ( memberName == "y" )
	{
		return ZephyrValue( m_position.y );
	}

	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
void ZephyrPosition::GetDistFromOrigin( ZephyrArgs* args )
{
	args->SetValue( "dist", m_position.GetLength() );
}
