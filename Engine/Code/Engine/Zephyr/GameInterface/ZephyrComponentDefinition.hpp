#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrScriptDefinition;


//-----------------------------------------------------------------------------------------------
class ZephyrComponentDefinition
{
	friend class ZephyrComponent;

public:
	explicit ZephyrComponentDefinition( const std::string& entityType, const XmlElement& scriptElem );
	virtual ~ZephyrComponentDefinition() {}

	void									ReloadZephyrScriptDefinition();
		
	ZephyrScriptDefinition*					GetZephyrScriptDefinition() const							{ return m_zephyrScriptDef; }
	ZephyrValueMap							GetZephyrScriptInitialValues() const						{ return m_zephyrScriptInitialValues; }
	std::vector<EntityVariableInitializer>	GetZephyrEntityVarInits() const								{ return m_zephyrEntityVarInits; }

protected:
	bool									m_isScriptValid = false; // ?

	std::string								m_zephyrScriptName;				// Used for reloading
	ZephyrScriptDefinition*					m_zephyrScriptDef = nullptr;
	ZephyrValueMap							m_zephyrScriptInitialValues;
	std::vector<EntityVariableInitializer>  m_zephyrEntityVarInits;
};
