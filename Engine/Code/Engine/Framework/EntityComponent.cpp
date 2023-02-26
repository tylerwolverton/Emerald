#include "Engine/Framework/EntityComponent.hpp"
#include "Engine/Framework/Entity.hpp"


//-----------------------------------------------------------------------------------------------
EntityComponentId EntityComponent::s_nextComponentId = 1000;
std::map<EntityComponentId, std::string> EntityComponent::s_componentTypeNames;


//-----------------------------------------------------------------------------------------------
EntityComponent::EntityComponent( const EntityId& parentEntityId )
	: m_parentEntityId( parentEntityId )
{
	m_id = s_nextComponentId++;
}


//-----------------------------------------------------------------------------------------------
std::string EntityComponent::GetParentEntityName() const
{
	if ( m_parentEntityId == INVALID_ENTITY_ID )
	{
		return "Unknown";
	}

	return Entity::GetName( m_parentEntityId );
}


//-----------------------------------------------------------------------------------------------
std::string EntityComponent::GetTypeName() const
{
	return s_componentTypeNames[m_typeId];
}


//-----------------------------------------------------------------------------------------------
void EntityComponent::SetTypeName( const std::string& name )
{
	s_componentTypeNames[m_typeId] = name;
}
