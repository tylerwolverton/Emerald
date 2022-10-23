#pragma once
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
class Entity
{
public:
	Entity();

	EntityId	GetId() const { return m_id; }
	std::string	GetName() const;
	void		SetName( const std::string& name );

	static std::string GetName( const EntityId& id );

protected:
	EntityId		m_id = INVALID_ENTITY_ID;

	// Statics
	static EntityId	s_nextEntityId;
	static std::map<EntityId, std::string> s_entityNames;
};
