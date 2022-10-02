#pragma once
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
class Entity
{
public:
	Entity();

	EntityId GetId() const { return m_id; }

protected:
	EntityId		m_id = -1;

	// Statics
	static EntityId	s_nextEntityId;
};
