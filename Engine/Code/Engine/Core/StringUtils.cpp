#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/Polygon2.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <stdarg.h>
#include <cctype>


//-----------------------------------------------------------------------------------------------
const int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringv( char const* format, va_list args )
{
	char buffer[STRINGF_STACK_LOCAL_TEMP_LENGTH];
	vsnprintf_s( buffer, STRINGF_STACK_LOCAL_TEMP_LENGTH, format, args );
	return buffer;
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const char* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( const int maxLength, const char* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}


//-----------------------------------------------------------------------------------------------
bool atob( const char* value )
{
	if ( !_stricmp( value, "T" )
		 || !_stricmp( value, "true" )
		 || !_stricmp( value, "1" ) )
	{
		return true;
	}
	else if ( !_stricmp( value, "F" )
			|| !_stricmp( value, "false" )
			|| !_stricmp( value, "0" ) )
	{
		return false;
	}

	ERROR_AND_DIE( Stringf( "Tried to convert '%s' to bool!", value ) );
}


//-----------------------------------------------------------------------------------------------
bool atob( const std::string& value )
{
	return atob( value.c_str() );
}


//-----------------------------------------------------------------------------------------------
std::vector<std::string> SplitStringOnDelimiter( const std::string& stringToSplit, char delimiter )
{
	std::vector<std::string> splitStrings;

	size_t startPos = 0;
	size_t foundPos = 0;
	while ( foundPos != std::string::npos )
	{
		foundPos = stringToSplit.find( delimiter, startPos );

		splitStrings.push_back( stringToSplit.substr( startPos, foundPos - startPos ) );
		startPos = foundPos + 1;
	}

	return splitStrings;
}


//-----------------------------------------------------------------------------------------------
bool IsWhitespace( unsigned char c )
{
	return std::isspace( c ) != 0;
}


//-----------------------------------------------------------------------------------------------
bool IsEmptyOrWhitespace( const std::string& stringToCheck )
{
	if ( stringToCheck.size() == 0 )
	{
		return true;
	}

	for ( uint stringIdx = 0; stringIdx < stringToCheck.size() - 1; ++stringIdx )
	{
		if ( !IsWhitespace( stringToCheck[stringIdx] ) )
		{
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
std::string TrimOuterWhitespace( const std::string& stringToTrim )
{
	if ( stringToTrim.size() == 0 )
	{
		return "";
	}

	size_t startPos = 0;
	
	while ( startPos < stringToTrim.size() - 1
			&& IsWhitespace( stringToTrim[startPos] ) )
	{
		++startPos;
	}  

	size_t endPos = stringToTrim.size() - 1;

	while ( endPos > startPos
			&& IsWhitespace( stringToTrim[endPos] ) )
	{
		--endPos;
	}
	
	// Move endpos back to the first whitespace char
	++endPos;
	return stringToTrim.substr( startPos, endPos - startPos );
}


//-----------------------------------------------------------------------------------------------
Strings TrimOuterWhitespace( const Strings& stringsToTrim )
{
	Strings trimmedStrings;
	trimmedStrings.reserve( stringsToTrim.size() );

	for( uint stringIdx = 0; stringIdx < stringsToTrim.size(); ++stringIdx )
	{
		std::string trimmedStr = TrimOuterWhitespace( stringsToTrim[stringIdx] );
		if ( trimmedStr != "" )
		{
			trimmedStrings.push_back( trimmedStr );
		}
	}

	return trimmedStrings;
}


//-----------------------------------------------------------------------------------------------
int ConvertStringToInt( const std::string& string )
{
	return atoi( string.c_str() );
}


//-----------------------------------------------------------------------------------------------
Ints ConvertStringsToInts( const Strings& strings )
{
	Ints ints;
	ints.reserve( strings.size() );

	for ( uint stringIdx = 0; stringIdx < strings.size() - 1; ++stringIdx )
	{
		ints.push_back( ConvertStringToInt( strings[stringIdx].c_str() ) );
	}

	return ints;
}


//-----------------------------------------------------------------------------------------------
float ConvertStringToFloat( const std::string& string )
{
	return (float)atof( string.c_str() );
}


//-----------------------------------------------------------------------------------------------
Floats ConvertStringsToFloats( const Strings& strings )
{
	Floats floats;
	floats.reserve( strings.size() );

	for ( uint stringIdx = 0; stringIdx < strings.size() - 1; ++stringIdx )
	{
		floats.push_back( ConvertStringToFloat( strings[stringIdx].c_str() ) );
	}

	return floats;
}


//-----------------------------------------------------------------------------------------------
bool IsEqualIgnoreCase( const std::string& a, const std::string& b )
{
	return !_stricmp( a.c_str(), b.c_str() );
}


//-----------------------------------------------------------------------------------------------
bool IsEqualIgnoreCase( const char* a, const std::string& b )
{
	return !_stricmp( a, b.c_str() );
}


//-----------------------------------------------------------------------------------------------
bool IsEqualIgnoreCase( const std::string& a, const char* b )
{
	return !_stricmp( a.c_str(), b );
}


//-----------------------------------------------------------------------------------------------
bool IsEqualIgnoreCase( const char* a, const char* b )
{
	return !_stricmp( a, b );
}


//-----------------------------------------------------------------------------------------------
template<> std::string ToString( bool value )					{ return value ? "true" : "false"; }
template<> std::string ToString( int value )					{ return std::string( Stringf( "%i", value ) ); }
template<> std::string ToString( uint value )					{ return std::string( Stringf( "%u", value ) ); }
template<> std::string ToString( float value )					{ return std::string( Stringf( "%.2f", value ) ); }
template<> std::string ToString( double value )					{ return std::string( Stringf( "%d", value ) ); }
template<> std::string ToString( const std::string& value )		{ return value; }
template<> std::string ToString( std::string value )			{ return value; }
template<> std::string ToString( char* value )					{ return value; }
template<> std::string ToString( void* value )					{ UNUSED (value); ERROR_AND_DIE( "Can't call ToString on void*" ); }
template<> std::string ToString( ZephyrValue value )			{ return value.EvaluateAsString(); }
template<> std::string ToString( ZephyrTypeBase* value )			{ UNUSED (value); ERROR_AND_DIE( "Can't call ToString on ZephyrType*" ); }


//-----------------------------------------------------------------------------------------------
template<> bool FromString( const std::string& value, bool defaultValue )
{
	UNUSED( defaultValue );

	return atob( value.c_str() );
}


//-----------------------------------------------------------------------------------------------
template<> int FromString( const std::string& value, int defaultValue )
{
	UNUSED( defaultValue );

	return atoi( value.c_str() );
}


//-----------------------------------------------------------------------------------------------
template<> uint FromString( const std::string& value, uint defaultValue )
{
	int intVal = atoi( value.c_str() );

	if ( intVal < 0 )
	{
		return defaultValue;
	}
	else
	{
		return (uint)intVal;
	}
}


//-----------------------------------------------------------------------------------------------
template<> float FromString( const std::string& value, float defaultValue )
{
	UNUSED( defaultValue );

	return (float)atof( value.c_str() );
}


//-----------------------------------------------------------------------------------------------
template<> double FromString( const std::string& value, double defaultValue )
{
	UNUSED( defaultValue );

	return atof( value.c_str() );
}


//-----------------------------------------------------------------------------------------------
template<> std::string FromString( const std::string& value, std::string defaultValue )
{
	UNUSED( defaultValue );

	return value;
}


//-----------------------------------------------------------------------------------------------
template<> const char* FromString( const std::string& value, const char* defaultValue )
{
	UNUSED( defaultValue );

	return value.c_str();
}


//-----------------------------------------------------------------------------------------------
template<> void* FromString( const std::string& value, void* defaultValue )
{
	UNUSED( value );
	UNUSED( defaultValue );

	ERROR_AND_DIE( "Saving pointers as string values in NamedProperties is not supported." );

	//return nullptr;
}


//-----------------------------------------------------------------------------------------------
template<> ZephyrValue FromString( const std::string& value, ZephyrValue defaultValue )
{
	UNUSED( value );
	UNUSED( defaultValue );

	ERROR_AND_DIE( "Serialization of ZephyrValues isn't fully implemented yet." );

	//ZephyrValue newValue;
	//newValue.DeserializeFromString( value );
	//return newValue;
}


//-----------------------------------------------------------------------------------------------
template<> ZephyrTypeBase* FromString( const std::string& value, ZephyrTypeBase* defaultValue )
{
	UNUSED( value );
	UNUSED( defaultValue );

	ERROR_AND_DIE( "Saving ZephyrType pointers as string values in NamedProperties is not supported." );

	//return nullptr;
}
