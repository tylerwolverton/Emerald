#pragma once
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
class EntityComponent
{
public:
	EntityComponent();

	EntityComponentId		GetId() const			{ return m_id; }
	EntityComponentTypeId	GetTypeId() const		{ return m_typeId; }
	
	std::string				GetTypeName() const;
	void					SetTypeName( const std::string& name );

protected:
	EntityComponentId		m_id = -1;
	EntityComponentTypeId	m_typeId = -1;

	// Statics
	static EntityComponentId	s_nextComponentId;
	static std::map<EntityComponentTypeId, std::string> s_componentTypeNames;
};
