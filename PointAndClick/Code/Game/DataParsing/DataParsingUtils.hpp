#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrValue;


//-----------------------------------------------------------------------------------------------
const std::string ParseZephyrVarInitsFromData( const XmlElement* globalVarElem, 
												ZephyrValueMap& out_zephyrScriptInitialValues, 
												std::vector<EntityVariableInitializer>& out_zephyrEntityVarInits );


//-----------------------------------------------------------------------------------------------
ZephyrValue CreateZephyrValueFromData( const std::string& typeName, const std::string& valueStr );
