#include "Game/DataParsing/DataParsingUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"


//-----------------------------------------------------------------------------------------------
const std::string ParseZephyrVarInitsFromData( const XmlElement* globalVarElem, 
												ZephyrValueMap& out_zephyrScriptInitialValues, 
												std::vector<EntityVariableInitializer>& out_zephyrEntityVarInits )
{
	while ( globalVarElem != nullptr )
	{
		std::string typeName = ParseXmlAttribute( *globalVarElem, "type", "" );
		std::string varName = ParseXmlAttribute( *globalVarElem, "var", "" );
		std::string valueStr = ParseXmlAttribute( *globalVarElem, "value", "" );
		if ( typeName.empty() )
		{
			return Stringf( "GlobalVar is missing a variable type" );
		}
		if ( varName.empty() )
		{
			return Stringf( "GlobalVar is missing a variable name");
		}
		if ( valueStr.empty() )
		{
			return Stringf( "GlobalVar is missing a variable value" );
		}

		if ( varName == PARENT_ENTITY_STR )
		{
			return Stringf( "GlobalVar cannot initialize reserved entity variable '%s'.", PARENT_ENTITY_STR.c_str() );
		}

		// Convert value to correct type and store in map
		ZephyrValue newValue = CreateZephyrValueFromData( typeName, valueStr );

		if ( newValue.GetType() == eValueType::ENTITY )
		{
			if ( newValue == ZephyrValue::ERROR_VALUE )
			{
				return Stringf( "GlobalVar '%s' has unsupported type '%s'", varName.c_str(), typeName.c_str() );
			}

			out_zephyrEntityVarInits.emplace_back( varName, valueStr );
		}
		else
		{
			out_zephyrScriptInitialValues[varName] = newValue;
		}

		globalVarElem = globalVarElem->NextSiblingElement( "GlobalVar" );
	}

	return "";
}


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
		// This needs to be handled by the caller
		return ZephyrValue( (EntityId)0 );
	}
	
	return ZephyrValue::ERROR_VALUE;
}
