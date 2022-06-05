#pragma once
#include "Engine/Core/EngineCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
struct Rgba8;
struct Vec2;
struct IntVec2;
struct Vec3;
struct OBB3;
class Polygon2;


//-----------------------------------------------------------------------------------------------
const std::string Stringv( char const* format, va_list args );
const std::string Stringf( const char* format, ... );
const std::string Stringf( const int maxLength, const char* format, ... );

bool atob( const char* value );
bool atob( const std::string& value );

std::vector<std::string> SplitStringOnDelimiter( const std::string& stringToSplit, char delimiter );

bool IsWhitespace( unsigned char c );
bool IsEmptyOrWhitespace( const std::string& stringToCheck );

std::string TrimOuterWhitespace( const std::string& stringToTrim );
Strings TrimOuterWhitespace( const Strings& stringsToTrim );

int ConvertStringToInt( const std::string& string );
Ints ConvertStringsToInts( const Strings& strings );
float ConvertStringToFloat( const std::string& string );
Floats ConvertStringsToFloats( const Strings& strings );

bool IsEqualIgnoreCase( const std::string& a, const std::string& b );
bool IsEqualIgnoreCase( const char* a, const std::string& b );
bool IsEqualIgnoreCase( const std::string& a, const char* b );
bool IsEqualIgnoreCase( const char* a, const char* b );


//-----------------------------------------------------------------------------------------------
template<typename T>	std::string ToString( T value )						{ return value.ToString(); }
template<>				std::string ToString( bool value );
template<>				std::string ToString( int value );
template<>				std::string ToString( uint value );
template<>				std::string ToString( float value );
template<>				std::string ToString( double value );
template<>				std::string ToString( const std::string& value );
template<>				std::string ToString( std::string value );
template<>				std::string ToString( char* value );
template<>				std::string ToString( void* value );


//-----------------------------------------------------------------------------------------------
template<typename T> T FromString( const std::string& value, T defaultValue )
{
	UNUSED( defaultValue );

	T convertedValue;
	convertedValue.SetFromText( value.c_str() );
	return convertedValue;
}

template<> bool			FromString( const std::string& value, bool defaultValue );
template<> int			FromString( const std::string& value, int defaultValue );
template<> uint			FromString( const std::string& value, uint defaultValue );
template<> float		FromString( const std::string& value, float defaultValue );
template<> double		FromString( const std::string& value, double defaultValue );
template<> std::string	FromString( const std::string& value, std::string defaultValue );
template<> const char*	FromString( const std::string& value, const char* defaultValue );
template<> void*		FromString( const std::string& value, void* defaultValue );
