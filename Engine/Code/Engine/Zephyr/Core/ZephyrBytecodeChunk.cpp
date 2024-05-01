#include "Engine/Zephyr/Core/ZephyrBytecodeChunk.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/StringUtils.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk::ZephyrBytecodeChunk( const std::string& name, ZephyrBytecodeChunk* parent )
	: m_name( name )
	, m_parentChunk( parent )
{
	m_variableScopes.push_back( new ZephyrScope() );
	
	if ( parent != nullptr )
	{
		m_variableScopes[0]->parentScope = parent->GetChunkVariableScope();
	}
}


//-----------------------------------------------------------------------------------------------
ZephyrBytecodeChunk::~ZephyrBytecodeChunk()
{
	PTR_MAP_SAFE_DELETE( m_eventBytecodeChunks );
	PTR_VECTOR_SAFE_DELETE( m_variableScopes );
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
	return m_variableScopes[m_curScopeIdx]->TryToGetVariable(identifier, out_value);
}


//-----------------------------------------------------------------------------------------------
bool ZephyrBytecodeChunk::TryToGetVariableFromCurrentScope( const std::string& identifier, ZephyrValue& out_value ) const
{
	auto iter = m_variableScopes[m_curScopeIdx]->variables.find( identifier );
	if ( iter != m_variableScopes[m_curScopeIdx]->variables.end() )
	{
		out_value = iter->second;
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::SetScopeFromIdx( int newScopeIdx )
{
	m_curScopeIdx = newScopeIdx;
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
int ZephyrBytecodeChunk::PushVariableScope()
{
	ZephyrScope* newScope = new ZephyrScope();
	newScope->parentScope = m_variableScopes[m_curScopeIdx];
	m_variableScopes.push_back( newScope );
	
	m_curScopeIdx = (int)m_variableScopes.size() - 1;
	return m_curScopeIdx;
}


//-----------------------------------------------------------------------------------------------
int ZephyrBytecodeChunk::PopVariableScope()
{
	// TODO: Need to tell VM to reset vars when popping?

	if ( m_variableScopes.size() <= 1 )
	{
		ERROR_AND_DIE( Stringf( "Tried to pop too many variable scopes from %s", m_name.c_str() ) );
	}
	
	ZephyrScope* parentScope = m_variableScopes[m_curScopeIdx]->parentScope;

	for ( int scopeIdx = (int)m_variableScopes.size() - 1; scopeIdx >= 0; --scopeIdx )
	{
		if ( parentScope == m_variableScopes[scopeIdx] )
		{
			m_curScopeIdx = scopeIdx;
			return scopeIdx;
		}
	}

	return -1;
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::SetVariable( const std::string& identifier, const ZephyrValue& value )
{
	m_variableScopes[m_curScopeIdx]->SetVariable(identifier, value);
}


//-----------------------------------------------------------------------------------------------
void ZephyrBytecodeChunk::DefineVariable( const std::string& identifier, const ZephyrValue& value )
{
	m_variableScopes[m_curScopeIdx]->DefineVariable(identifier, value);
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
