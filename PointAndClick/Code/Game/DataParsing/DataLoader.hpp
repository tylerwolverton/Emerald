#pragma once
//-----------------------------------------------------------------------------------------------
class World;


//-----------------------------------------------------------------------------------------------
class DataLoader
{
public:
	static void LoadAllDataAssets( World& world );
	static void ReloadAllData( World& world );
	static void ReloadAllScripts( World& world );

private:
	static void LoadSounds();
	static void LoadMaps( World& world );
	static void LoadEntityTypes();
	static void LoadWorldDefinition( World& world );
	static void LoadAndCompileZephyrScripts();
};
