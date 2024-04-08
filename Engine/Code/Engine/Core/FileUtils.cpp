#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"

#include <io.h>
#include <iostream>
#include <fstream>


//-----------------------------------------------------------------------------------------------
void* FileReadToNewBuffer( const std::string& filename, uint32_t* out_fileSize )
{
	FILE* fp;
	fopen_s( &fp, filename.c_str(), "r" );
	if ( fp == nullptr )
	{
		return nullptr;
	}

	// get size of file
	fseek( fp, 0, SEEK_END );
	long fileSize = ftell( fp );

	byte* buffer = new byte[fileSize + 1];
	// Guarantee or die not null
	if ( buffer != nullptr )
	{
		fseek( fp, 0, SEEK_SET );
		uint32_t bytesRead = (uint32_t)fread( buffer, 1, fileSize, fp );
		buffer[bytesRead] = NULL;
	}

	if ( out_fileSize != nullptr )
	{
		*out_fileSize = (uint32_t)fileSize;
	}

	fclose( fp );

	return buffer;
}


//-----------------------------------------------------------------------------------------------
void* FileReadBinaryToNewBuffer( const std::string& filename, uint32_t* out_fileSize /*= nullptr */ )
{
	FILE* fp;
	fopen_s( &fp, filename.c_str(), "rb" );
	if ( fp == nullptr )
	{
		return nullptr;
	}

	// get size of file
	fseek( fp, 0, SEEK_END );
	long fileSize = ftell( fp );

	byte* buffer = new byte[fileSize];
	// Guarantee or die not null
	if ( buffer != nullptr )
	{
		fseek( fp, 0, SEEK_SET );
		fread( buffer, 1, fileSize, fp );
	}

	if ( out_fileSize != nullptr )
	{
		*out_fileSize = (uint32_t)fileSize;
	}

	fclose( fp );

	return buffer;
}


//-----------------------------------------------------------------------------------------------
bool WriteBufferToFile( const std::string& filename, byte* buffer, uint32_t bufferSize )
{
	FILE* fp;
	fopen_s( &fp, filename.c_str(), "wb" );
	if ( fp == nullptr )
	{
		return false;
	}

	fwrite( buffer, sizeof( byte ), bufferSize, fp );

	fclose( fp );

	return true;
}


//-----------------------------------------------------------------------------------------------
Strings SplitFileIntoLines( const std::string& filename )
{
	Strings lines;
	std::ifstream objFile;
	objFile.open( filename, std::ios::in );

	if ( !objFile.is_open() )
	{
		g_devConsole->PrintString( Stringf( "Couldn't open file to split: '%s'", filename.c_str(), Rgba8::RED ) );
		return lines;
	}

	std::string line;
	while ( std::getline( objFile, line ) )
	{
		lines.push_back( line );
	}

	objFile.close();

	return lines;
}


//-----------------------------------------------------------------------------------------------
void GetAllFilePathsInFolderRecursive( const std::string& relativeRootFolderPath, const char* filePattern, Strings& out_fileNames )
{
	std::string filePath = relativeRootFolderPath + "/*";
	std::string fileNamePattern = filePattern ? filePattern : "*";

	// Get all file names from subfolders
	_finddata_t fileInfo;
	intptr_t searchHandle = _findfirst( filePath.c_str(), &fileInfo );
	while ( searchHandle != -1 )
	{
		if ( !strcmp( fileInfo.name, "." )
			|| !strcmp( fileInfo.name, ".." ) )
		{
			// Ignore these
		}
		else if ( fileInfo.attrib & _A_SUBDIR )
		{
			std::string subDirectoryPath = relativeRootFolderPath + "/" + fileInfo.name;
			GetAllFilePathsInFolderRecursive( subDirectoryPath, filePattern, out_fileNames );
		}
		// Any file with other attributes will be considered a valid file for return
		else
		{
			std::string fileName( relativeRootFolderPath + "/" + fileInfo.name );

			if ( fileNamePattern == "*"
				|| fileName.ends_with( fileNamePattern ) )
			{
				out_fileNames.push_back( fileName );
			}
		}

		int errorCode = _findnext( searchHandle, &fileInfo );
		if ( errorCode != 0 )
		{
			break;
		}
	}
}


//-----------------------------------------------------------------------------------------------
Strings GetAllFileNamesInFolder( const std::string& relativeFolderPath, const char* filePattern )
{
	Strings fileNamesInFolder;

	std::string fileNamePattern = filePattern ? filePattern : "*";
	std::string filePath = relativeFolderPath + "/" + fileNamePattern;

	_finddata_t fileInfo;
	intptr_t searchHandle = _findfirst( filePath.c_str(), &fileInfo );
	while ( searchHandle != -1 )
	{
		fileNamesInFolder.push_back( fileInfo.name );
		int errorCode = _findnext( searchHandle, &fileInfo );
		if ( errorCode !=0 )
		{
			break;
		}
	}

	return fileNamesInFolder;
}


//-----------------------------------------------------------------------------------------------
std::string GetFileName( const std::string& filePath )
{
	size_t lastSlashPos = filePath.find_last_of( "/" );
	size_t lastOtherSlashPos = filePath.find_last_of( "\\" );
	if ( lastSlashPos == std::string::npos ||
		 lastSlashPos < lastOtherSlashPos && lastOtherSlashPos != std::string::npos )
	{
		lastSlashPos = lastOtherSlashPos;
	}

	if ( lastSlashPos == std::string::npos )
	{
		lastSlashPos = 0;
	}
	else
	{
		++lastSlashPos;
	}

	return filePath.substr( lastSlashPos, filePath.size() - 1 );
}


//-----------------------------------------------------------------------------------------------
std::string GetFileNameWithoutExtension( const std::string& filePath )
{
	size_t firstCharPos = 0;
	size_t numCharsToRead = filePath.size();

	size_t lastSlashPos = filePath.find_last_of( "/" );
	if ( lastSlashPos != std::string::npos )
	{
		firstCharPos = lastSlashPos + 1;
	}

	size_t extensionPos = filePath.find( "." );
	if ( extensionPos != std::string::npos )
	{
		numCharsToRead = extensionPos - firstCharPos;
	}
	
	return filePath.substr( firstCharPos, numCharsToRead );
}


//-----------------------------------------------------------------------------------------------
std::string GetFileExtension( const std::string& filePath )
{
	size_t extensionPos = filePath.find( "." );

	if ( extensionPos == std::string::npos )
	{
		return "";
	}
	
	return filePath.substr( extensionPos );
}


