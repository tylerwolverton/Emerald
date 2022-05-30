#pragma once
#include <map>


//-----------------------------------------------------------------------------------------------
template< typename BaseObject, typename CreatorId, typename CreationFunc, typename ConstructionParam >
class ObjectFactory
{
public:
	bool RegisterCreator( const CreatorId& id, CreationFunc creationFunc )
	{
		return m_creatorMap.insert( std::make_pair<CreatorId, CreationFunc>( id, creationFunc ) ).second;
	}

	bool UnRegisterCreator( const CreatorId& id )
	{
		return m_creatorMap.erase( id ) == 1;
	}

	BaseObject* CreateObject( const CreatorId& id, const ConstructionParam& params )
	{
		const auto creatorIter = m_creatorMap.find( id );
		if ( creatorIter == m_creatorMap.end() )
		{
			// Log error
			return nullptr;
		}

		return creatorIter->second( params );
	}

private:
	std::map<CreatorId, CreationFunc> m_creatorMap;
};
