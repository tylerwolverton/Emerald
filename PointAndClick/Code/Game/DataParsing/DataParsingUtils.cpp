#include "Game/DataParsing/DataParsingUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrValue CreateZephyrValueFromData( const std::string& typeName, const std::string& valueStr )
{
	if ( !_strcmpi( typeName.c_str(), "string" ) )
	{
		return ZephyrValue( valueStr );
	}
	else if ( !_strcmpi( typeName.c_str(), "number" ) )
	{
		return ZephyrValue( FromString( valueStr, 0.f ) );
	}
	else if ( !_strcmpi( typeName.c_str(), "bool" ) )
	{
		return ZephyrValue( FromString( valueStr, false ) );
	}
	else if ( !_strcmpi( typeName.c_str(), "vec2" ) )
	{
		return ZephyrValue( FromString( valueStr, Vec2::ZERO ) );
	}
	else if ( !_strcmpi( typeName.c_str(), "vec3" ) )
	{
		return ZephyrValue( FromString( valueStr, Vec3::ZERO ) );
	}
	else if ( !_strcmpi( typeName.c_str(), "entity" ) )
	{
		// This needs to be hooked up by the caller
		return ZephyrValue( (EntityId)0 );
	}
	
	return ZephyrValue::ERROR_VALUE;
}
