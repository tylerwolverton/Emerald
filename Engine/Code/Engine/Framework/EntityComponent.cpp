#include "Engine/Framework/EntityComponent.hpp"


//-----------------------------------------------------------------------------------------------
EntityComponentId EntityComponent::s_nextComponentId = 1000;
std::map<EntityComponentId, std::string> EntityComponent::s_componentTypeNames;


//-----------------------------------------------------------------------------------------------
EntityComponent::EntityComponent()
{
	m_id = s_nextComponentId++;
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
