#include "Game/DataParsing/DataLoader.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Zephyr/Core/ZephyrCompiler.hpp"
#include "Engine/Zephyr/Core/ZephyrScriptDefinition.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponentDefinition.hpp"

#include "Game/Core/GameCommon.hpp"
#include "Game/DataParsing/EntityTypeDefinition.hpp"
#include "Game/DataParsing/EntityTypesParser.hpp"
#include "Game/DataParsing/MapDefinition.hpp"
#include "Game/DataParsing/WorldDefinitionParser.hpp"
#include "Game/Framework/Game.hpp"
#include "Game/Framework/World.hpp"


//-----------------------------------------------------------------------------------------------
void DataLoader::LoadAllDataAssets( World& world )
{
	LoadSounds();
	LoadAndCompileZephyrScripts();
	LoadEntityTypes();
	LoadWorldDefinition( world );
	LoadMaps( world );
}


//-----------------------------------------------------------------------------------------------
void DataLoader::ReloadAllScripts( World& world )
{
	world.UnloadAllEntityScripts();

	PTR_MAP_SAFE_DELETE( ZephyrScriptDefinition::s_definitions );

	LoadAndCompileZephyrScripts();

	for ( auto& entityDef : EntityTypeDefinition::s_definitions )
	{
		if ( entityDef.second != nullptr && entityDef.second->GetZephyrCompDef() != nullptr )
		{
			entityDef.second->GetZephyrCompDef()->ReloadZephyrScriptDefinition();
		}
	}

	world.ReloadAllEntityScripts();
}


//-----------------------------------------------------------------------------------------------
void DataLoader::ReloadAllData( World& world )
{
	// Unload
	world.Reset();

	g_gameConfigBlackboard.Clear();

	PTR_MAP_SAFE_DELETE( ZephyrScriptDefinition::s_definitions );
	PTR_MAP_SAFE_DELETE( EntityTypeDefinition::s_definitions );
	PTR_VECTOR_SAFE_DELETE( SpriteSheet::s_definitions );

	// Load
	PopulateGameConfig();

	LoadAllDataAssets( world );
}


//-----------------------------------------------------------------------------------------------
void DataLoader::LoadSounds()
{
	std::string folderRoot( g_gameConfigBlackboard.GetValue( "audioRoot", "" ) );

	Strings audioFiles;
	GetAllFilePathsInFolderRecursive( folderRoot, "*", audioFiles );
	for ( int soundIdx = 0; soundIdx < (int)audioFiles.size(); ++soundIdx )
	{
		g_audioSystem->CreateOrGetSound( audioFiles[soundIdx] );
	}
}


//-----------------------------------------------------------------------------------------------
void DataLoader::LoadMaps( World& world )
{
	std::string folderRoot( g_gameConfigBlackboard.GetValue( "mapsRoot", "" ) );

	Strings mapFiles;
	GetAllFilePathsInFolderRecursive( folderRoot, ".xml", mapFiles );
	for ( int mapIdx = 0; mapIdx < (int)mapFiles.size(); ++mapIdx )
	{
		MapDefinition mapData( mapFiles[mapIdx] );
		if ( mapData.isValid )
		{
			world.AddNewMap( mapData );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void DataLoader::LoadEntityTypes()
{
	std::string entityTypesPath( g_gameConfigBlackboard.GetValue( "entityTypesPath", "" ) );

	EntityTypesParser::ParseFromFile( entityTypesPath );
}


//-----------------------------------------------------------------------------------------------
void DataLoader::LoadWorldDefinition( World& world )
{
	std::string filePath( g_gameConfigBlackboard.GetValue( "worldDefPath", "" ) );

	std::vector<EntitySpawnParams> worldEntitiesSpawnParams = WorldDefinitionParser::ParseEntitiesFromFile( filePath );
	for ( const auto& worldEntitySpawnParams : worldEntitiesSpawnParams )
	{
		world.SpawnNewWorldEntity( worldEntitySpawnParams );
	}
}


//-----------------------------------------------------------------------------------------------
void DataLoader::LoadAndCompileZephyrScripts()
{
	std::string folderRoot( g_gameConfigBlackboard.GetValue( "scriptsRoot", "" ) );

	Strings scriptFiles;
	GetAllFilePathsInFolderRecursive( folderRoot, ".zephyr", scriptFiles );
	//Strings scriptFiles = GetFileNamesInFolder( folderRoot, "*.zephyr" );
	for ( int scriptIdx = 0; scriptIdx < (int)scriptFiles.size(); ++scriptIdx )
	{
		std::string& scriptName = scriptFiles[scriptIdx];

		//std::string scriptFullPath( folderRoot );
		//scriptFullPath += "/";
		//scriptFullPath += scriptName;

		// Save compiled script into static map
		ZephyrScriptDefinition* scriptDef = ZephyrCompiler::CompileScriptFile( scriptName );
		scriptDef->m_name = scriptName;

		ZephyrScriptDefinition::s_definitions[scriptName] = scriptDef;
	}
}
