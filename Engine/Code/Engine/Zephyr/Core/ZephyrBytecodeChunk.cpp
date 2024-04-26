#include "Engine/Zephyr/Core/ZephyrBytecodeChunk.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk::ZephyrBytecodeChunk( const std::string& name, ZephyrBytecodeChunk* parent )
	: m_name( name )
	, m_parentChunk( parent )
{
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk::~ZephyrBytecodeChunk()
{
	PTR_MAP_SAFE_DELETE( m_eventBytecodeChunks );
}


//-----------------------------------------------------------------------------------------------
std::string ZephyrBytecodeChunk::GetFullyQualifiedName() const
{
	std::string fullName;
	if ( m_parentChunk != nullptr )
	{
		fullName.append( m_parentChunk->GetName() );
		fullName.append( "::" );
	}

	fullName.append( GetName() );
	return fullName;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrBytecodeChunk::TryToGetVariable( const std::string& identifier, ZephyrValue& out_value ) const
{
	return m_variableScope.TryToGetVariable( identifier, out_value );
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::WriteByte( byte newByte )
{
	m_bytes.push_back( newByte );
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::WriteByte( eOpCode opCode )
{
	WriteByte( (byte)opCode );
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::WriteByte( int constantIdx )
{
	//if ( constantIdx < 0
	//	 || constantIdx > sizeof( byte ) - 1 )
	//{
	//	// Print error?
	//	return;
	//}

	WriteByte( (byte)constantIdx );
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::WriteConstant( const ZephyrValue& constant )
{
	WriteByte( AddConstant( constant ) );
}


//-----------------------------------------------------------------------------------------------
int ZephyrBytecodeChunk::AddConstant( const ZephyrValue& constant )
{
	m_constants.push_back( constant );
	return (int)m_constants.size() - 1;
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::SetConstantAtIdx( int idx, const ZephyrValue& constant )
{
	if ( idx > (int)m_constants.size() - 1 )
	{
		g_devConsole->PrintWarning( "Tried to write to constant outside bounds of constant vector" );
		return;
	}

	m_constants[idx] = constant;
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::AddEventChunk( ZephyrBytecodeChunk* eventBytecodeChunk )
{
	m_eventBytecodeChunks[eventBytecodeChunk->GetName()] = eventBytecodeChunk;
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::SetVariable( const std::string& identifier, const ZephyrValue& value )
{
	m_variableScope.SetVariable( identifier, value );
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::DefineVariable( const std::string& identifier, const ZephyrValue& value )
{
	m_variableScope.DefineVariable( identifier, value );
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::Disassemble() const
{
	int byteIdx = 0;
	int opNum = 0;
	while ( byteIdx < (int)m_bytes.size() )
	{
		byte const& instruction = GetByte( byteIdx++ );
		eOpCode opCode = ByteToOpCode( instruction );
		std::string instructionLine = Stringf( "%i: %s", opNum, ToString( opCode ).c_str() );

		switch ( opCode )
		{
			case eOpCode::CONSTANT:
			{
				int constIdx = GetByte( byteIdx++ );
				instructionLine += Stringf( " %i", constIdx );
			}
			break;
		}

		g_devConsole->PrintString( instructionLine );
		++opNum;
	}
}


//-----------------------------------------------------------------------------------------------
std::string ToString( eBytecodeChunkType type )
{
	switch ( type )
	{
		case eBytecodeChunkType::GLOBAL:		return "Global State";
		case eBytecodeChunkType::STATE:			return "State";
		case eBytecodeChunkType::EVENT:			return "Function";
	}

	return "Unknown";
}
