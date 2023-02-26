#pragma once
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
class EntityComponent
{
public:
	EntityComponent( const EntityId& parentEntityId );

	EntityComponentId		GetId() const				{ return m_id; }
	EntityComponentTypeId	GetTypeId() const			{ return m_typeId; }
	EntityId				GetParentEntityId() const	{ return m_parentEntityId; }
	std::string				GetParentEntityName() const;

	std::string				GetTypeName() const;
	void					SetTypeName( const std::string& name );

protected:
	EntityComponentId		m_id = INVALID_ENTITY_COMPONENT_ID;
	EntityComponentTypeId	m_typeId = ENTITY_COMPONENT_TYPE_INVALID;
	EntityId				m_parentEntityId = INVALID_ENTITY_ID;

	// Statics
	static EntityComponentId	s_nextComponentId;
	static std::map<EntityComponentTypeId, std::string> s_componentTypeNames;
};
