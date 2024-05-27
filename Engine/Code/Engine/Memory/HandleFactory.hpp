#pragma once
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Memory/SmartPtr.hpp"

#include <map>


//-----------------------------------------------------------------------------------------------
template< typename BaseObject, typename CreatorId, typename CreationFunc, typename ConstructionParams >
class HandleFactory
{
public:
	bool RegisterCreator( CreatorId id, CreationFunc creationFunc )
	{
		m_creatorMap[id] = creationFunc;
		return m_creatorMap[id];
	}

	bool UnRegisterCreator( const CreatorId& id )
	{
		return m_creatorMap.erase( id ) == 1;
	}

	Handle<BaseObject> CreateHandle( CreatorId id, ConstructionParams params )
	{
		const auto creatorIter = m_creatorMap.find( id );
		if ( creatorIter == m_creatorMap.end() )
		{
			ERROR_AND_DIE( Stringf( "ObjectFactory: Can't find a creation function with id '%s'", id.c_str() ) );
		}

		return creatorIter->second( params );
	}

private:
	std::map<CreatorId, CreationFunc> m_creatorMap;
};
