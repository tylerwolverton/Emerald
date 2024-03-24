#pragma once
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <map>
#include <string>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;


//-----------------------------------------------------------------------------------------------
class ZephyrScriptDefinition
{
public:
	ZephyrScriptDefinition( ZephyrBytecodeChunk* globalBytecodeChunk, const ZephyrBytecodeChunkMap& bytecodeChunks, const std::map<std::string, std::vector<int>>& bytecodeOpCodeToLineNumMap );
	~ZephyrScriptDefinition();

	bool IsValid() const																	{ return m_isValid; }
	void SetIsValid( bool isValid )															{ m_isValid = isValid; }

	ZephyrBytecodeChunk* GetGlobalBytecodeChunk() const										{ return m_globalBytecodeChunk; }
	ZephyrBytecodeChunk* GetBytecodeChunkByName( const std::string& name ) const;
	ZephyrBytecodeChunk* GetFirstStateBytecodeChunk() const;
	ZephyrBytecodeChunkMap GetAllStateBytecodeChunks() const;
	ZephyrBytecodeChunkMap GetAllEventBytecodeChunks() const;

	int GetLineNumFromOpCodeIdx( const std::string& bytecodeChunkName, int opCodeIdx ) const;

	static ZephyrScriptDefinition* GetZephyrScriptDefinitionByPath( const std::string& scriptPath );
	static ZephyrScriptDefinition* GetZephyrScriptDefinitionByName( const std::string& scriptName );

public:
	static std::map< std::string, ZephyrScriptDefinition* > s_definitions;

	// TEMP
	std::string m_name; // HashedString

private:
	bool m_isValid = false;
	static std::string s_dataPathSuffix;

	ZephyrBytecodeChunk* m_globalBytecodeChunk = nullptr;					// Owned by ZephyrScriptDefinition
	ZephyrBytecodeChunkMap m_bytecodeChunks;								// Owned by ZephyrScriptDefinition

	std::map<std::string, std::vector<int>> m_bytecodeOpCodeToLineNumMap;
};
