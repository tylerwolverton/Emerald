#include "Engine/Core/NamedProperties.hpp"


//-----------------------------------------------------------------------------------------------
NamedProperties::~NamedProperties()
{
	std::map<std::string, TypedPropertyBase*>::iterator mapIter;

	for ( mapIter = m_keyValuePairs.begin(); mapIter != m_keyValuePairs.end(); mapIter++ )
	{
		delete( mapIter->second );
		mapIter->second = nullptr;
	}

	m_keyValuePairs.clear();
}


//-----------------------------------------------------------------------------------------------
void NamedProperties::PopulateFromXmlAttributes( const XmlElement& element )
{
	const XmlAttribute* attr = element.FirstAttribute();

	while ( attr != nullptr )
	{
		SetValue( attr->Name(), std::string( attr->Value() ) );
		attr = attr->Next();
	}
}


//-----------------------------------------------------------------------------------------------
void NamedProperties::SetValue( const std::string& keyname, const char* value )
{
	SetValue<std::string>( keyname, value );
}


//-----------------------------------------------------------------------------------------------
std::string NamedProperties::GetValue( const std::string& keyName, const char* defaultValue ) const
{
	return GetValue<std::string>( keyName, defaultValue );
}


//-----------------------------------------------------------------------------------------------
void NamedProperties::Clear()
{
	m_keyValuePairs.clear();
}


//-----------------------------------------------------------------------------------------------
TypedPropertyBase* NamedProperties::FindInMap( const std::string& key ) const
{
	auto iter = m_keyValuePairs.find( key );
	if ( iter != m_keyValuePairs.end() )
	{
		return iter->second;
	}
	else
	{
		return nullptr;
	}
}
