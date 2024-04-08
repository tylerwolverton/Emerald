#include <string>
#include "Engine/Core/EngineCommon.hpp"


//-----------------------------------------------------------------------------------------------
void* FileReadToNewBuffer( const std::string& filename, uint32_t* out_fileSize = nullptr );
void* FileReadBinaryToNewBuffer( const std::string& filename, uint32_t* out_fileSize = nullptr );
bool  WriteBufferToFile( const std::string& filename, byte* buffer, uint32_t bufferSize );

Strings SplitFileIntoLines( const std::string& filename );
void GetAllFilePathsInFolderRecursive( const std::string& relativeRootFolderPath, const char* filePattern, Strings& out_fileNames );
Strings GetAllFileNamesInFolder( const std::string& relativeFolderPath, const char* filePattern );
std::string GetFileName( const std::string& filePath );
std::string GetFileNameWithoutExtension( const std::string& filePath );
std::string GetFileExtension( const std::string& filePath );
