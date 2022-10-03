#include "Engine/Framework/Entity.hpp"


//-----------------------------------------------------------------------------------------------
EntityId Entity::s_nextEntityId = 1000;
std::map<EntityId, std::string> Entity::s_entityNames;


//-----------------------------------------------------------------------------------------------
Entity::Entity()
{
	m_id = s_nextEntityId++;
	s_entityNames[m_id] = "Unknown";
}


//-----------------------------------------------------------------------------------------------
std::string Entity::GetName() const
{
	return s_entityNames[m_id];
}


//-----------------------------------------------------------------------------------------------
void Entity::SetName( const std::string& name )
{
	s_entityNames[m_id] = name;
}
