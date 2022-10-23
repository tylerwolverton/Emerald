#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/HashedString.hpp"

#include <string>

class ZephyrEntity;

typedef void ( *EventCallbackFunctionPtrType )( EventArgs* );

struct ZephyrEvent
{
public:
	HashedString name;
	std::vector<EventCallbackFunctionPtrType> callbacks;
};


class ZephyrEventSystem
{
public:
	void RegisterGlobalEvent( const std::string& name, EventCallbackFunctionPtrType callback );

	// TODO: Replace with id
	void FireEvent( ZephyrEntity* targetEntity, const std::string eventName, EventArgs* args );
	void FireEvent( const std::string eventName, EventArgs* args );

private:
	std::vector<ZephyrEvent> m_globalEvents;
};
