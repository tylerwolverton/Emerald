#pragma once
#include <map>


//-----------------------------------------------------------------------------------------------
template< typename BaseObject, typename CreatorId, typename CreationFunc, typename ConstructionParams >
class ObjectFactory
{
public:
	bool RegisterCreator( CreatorId id, CreationFunc creationFunc )
	{
		return m_creatorMap.insert( std::make_pair<CreatorId, CreationFunc>( id, creationFunc ) ).second;
	}

	bool UnRegisterCreator( const CreatorId& id )
	{
		return m_creatorMap.erase( id ) == 1;
	}

	BaseObject* CreateObject( CreatorId id, ConstructionParams params )
	{
		const auto creatorIter = m_creatorMap.find( id );
		if ( creatorIter == m_creatorMap.end() )
		{
			// Log error... or leave to caller
			return nullptr;
		}

		return creatorIter->second( params );
	}

private:
	std::map<CreatorId, CreationFunc> m_creatorMap;
};
