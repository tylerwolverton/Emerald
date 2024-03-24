#include "Engine/Zephyr/Core/ZephyrScriptDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Zephyr/Core/ZephyrBytecodeChunk.hpp"


//-----------------------------------------------------------------------------------------------
// Static Definitions
std::map< std::string, ZephyrScriptDefinition* > ZephyrScriptDefinition::s_definitions;
std::string ZephyrScriptDefinition::s_dataPathSuffix;


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition::ZephyrScriptDefinition( ZephyrBytecodeChunk* globalBytecodeChunk,
												const ZephyrBytecodeChunkMap& bytecodeChunks, 
												const std::map<std::string, std::vector<int>>& bytecodeOpCodeToLineNumMap )
	: m_globalBytecodeChunk( globalBytecodeChunk )
	, m_bytecodeChunks( bytecodeChunks )
	, m_bytecodeOpCodeToLineNumMap( bytecodeOpCodeToLineNumMap )
{
	s_dataPathSuffix = g_gameConfigBlackboard.GetValue( std::string( "dataPathSuffix" ), "" );
}


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition::~ZephyrScriptDefinition()
{
	PTR_MAP_SAFE_DELETE( m_bytecodeChunks );
	PTR_SAFE_DELETE( m_globalBytecodeChunk );
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk* ZephyrScriptDefinition::GetBytecodeChunkByName( const std::string& name ) const
{
	ZephyrBytecodeChunkMap::const_iterator  mapIter = m_bytecodeChunks.find( name );

	if ( mapIter == m_bytecodeChunks.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk* ZephyrScriptDefinition::GetFirstStateBytecodeChunk() const
{
	if ( m_bytecodeChunks.empty() )
	{
		return nullptr;
	}

	for ( auto chunk : m_bytecodeChunks )
	{
		if ( chunk.second->GetType() == eBytecodeChunkType::STATE
			 && chunk.second->IsInitialState() )
		{
			return chunk.second;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunkMap ZephyrScriptDefinition::GetAllStateBytecodeChunks() const
{
	ZephyrBytecodeChunkMap stateChunks;

	for ( auto chunk : m_bytecodeChunks )
	{
		if ( chunk.second->GetType() == eBytecodeChunkType::STATE )
		{
			stateChunks[chunk.first] = chunk.second;
		}
	}

	return stateChunks;
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunkMap ZephyrScriptDefinition::GetAllEventBytecodeChunks() const
{
	ZephyrBytecodeChunkMap eventChunks;

	for ( auto chunk : m_bytecodeChunks )
	{
		if ( chunk.second->GetType() == eBytecodeChunkType::EVENT )
		{
			eventChunks[chunk.first] = chunk.second;
		}
	}

	return eventChunks;
}


//-----------------------------------------------------------------------------------------------
int ZephyrScriptDefinition::GetLineNumFromOpCodeIdx( const std::string& bytecodeChunkName, int opCodeIdx ) const
{
	const auto& mapIter = m_bytecodeOpCodeToLineNumMap.find( bytecodeChunkName );
	if ( mapIter == m_bytecodeOpCodeToLineNumMap.cend() 
		|| opCodeIdx >= mapIter->second.size() )
	{
		return -1;
	}

	return mapIter->second[opCodeIdx];
}


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition* ZephyrScriptDefinition::GetZephyrScriptDefinitionByPath( const std::string& scriptPath )
{
	std::map< std::string, ZephyrScriptDefinition* >::const_iterator  mapIter = ZephyrScriptDefinition::s_definitions.find( scriptPath );

	if ( mapIter == s_definitions.cend() )
	{
		return nullptr;
	}

	return mapIter->second;
}


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition* ZephyrScriptDefinition::GetZephyrScriptDefinitionByName( const std::string& scriptName )
{
	std::string folderRoot( g_gameConfigBlackboard.GetValue( "scriptsRoot", "" ) );
	std::string fullPath = Stringf( "%s/%s", folderRoot.c_str(), scriptName.c_str() );

	return GetZephyrScriptDefinitionByPath( fullPath );
}
