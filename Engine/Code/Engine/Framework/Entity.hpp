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

protected:
	EntityId		m_id = -1;

	// Statics
	static EntityId	s_nextEntityId;
	static std::map<EntityId, std::string> s_entityNames;
};
