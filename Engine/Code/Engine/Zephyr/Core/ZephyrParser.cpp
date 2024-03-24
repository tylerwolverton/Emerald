#include "Engine/Zephyr/Core/ZephyrParser.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Zephyr/Core/ZephyrBytecodeChunk.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Zephyr/Core/ZephyrToken.hpp"
#include "Engine/Zephyr/Core/ZephyrScriptDefinition.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrEngineEvents.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"
#include "Engine/Zephyr/Types/ZephyrTypesCommon.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrParser::ZephyrParser( const std::string& filename, const std::vector<ZephyrToken>& tokens )
	: m_filename( filename )
	, m_tokens( tokens )
{
}


//-----------------------------------------------------------------------------------------------
ZephyrScriptDefinition* ZephyrParser::ParseTokensIntoScriptDefinition()
{
	CreateGlobalBytecodeChunk();

	ZephyrToken nextToken = GetCurToken();
	while ( !DoesTokenMatchType( nextToken, eTokenType::END_OF_FILE ) )
	{
		if ( !ParseStatement() )
		{
			return new ZephyrScriptDefinition( nullptr, m_bytecodeChunks, m_bytecodeOpCodeToLineNumMap );
		}

		nextToken = GetCurToken();
	}
	
	// Check for any chunks with too many constants
	bool anyErrorChunks = false;
	for ( auto const& bytecodeChunk : m_bytecodeChunks )
	{
		if ( bytecodeChunk.second->GetNumConstants() > 254 )
		{
			std::string bytecodeChunkType = ToString( bytecodeChunk.second->GetType() );
			ReportError( Stringf( "%s %s contains too many constants. Try to break up into smaller functions", bytecodeChunkType.c_str(), bytecodeChunk.first.c_str(), bytecodeChunkType.c_str() ) );
			anyErrorChunks = true;
		}

		for ( auto const& eventChunk : bytecodeChunk.second->GetEventBytecodeChunks() )
		{
			if ( eventChunk.second->GetNumConstants() > 254 )
			{
				std::string eventChunkType = ToString( eventChunk.second->GetType() );
				ReportError( Stringf( "%s %s contains too many constants. Try to break up into smaller functions", eventChunkType.c_str(), eventChunk.first.c_str(), eventChunkType.c_str() ) );
				anyErrorChunks = true;
			}
		}
	}

	if ( anyErrorChunks )
	{
		return new ZephyrScriptDefinition( nullptr, m_bytecodeChunks, m_bytecodeOpCodeToLineNumMap );
	}

	ZephyrScriptDefinition* validScript =  new ZephyrScriptDefinition( m_globalBytecodeChunk, m_bytecodeChunks, m_bytecodeOpCodeToLineNumMap );
	validScript->SetIsValid( true );

	return validScript;
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::CreateGlobalBytecodeChunk()
{
	m_globalBytecodeChunk = new ZephyrBytecodeChunk( "Global" );
	m_globalBytecodeChunk->SetType( eBytecodeChunkType::GLOBAL );

	m_curBytecodeChunk = m_globalBytecodeChunk;
	
	// Save reference to this entity into global state
	m_globalBytecodeChunk->SetVariable( PARENT_ENTITY_STR, ZephyrValue( INVALID_ENTITY_ID ) );

	m_curBytecodeChunksStack.push( m_globalBytecodeChunk );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::CreateBytecodeChunk( const std::string& chunkName, const eBytecodeChunkType& type )
{
	if ( type == eBytecodeChunkType::GLOBAL )
	{
		ReportError( "Each file can only have 1 Global definition" );
		return false;
	}

	switch ( m_curBytecodeChunk->GetType() )
	{
		case eBytecodeChunkType::GLOBAL:
		{
			// No error here, can define anything
		}
		break;

		case eBytecodeChunkType::STATE:
		{
			if ( type == eBytecodeChunkType::STATE )
			{
				ReportError( "Cannot define a State inside of another State definition" );
				return false;
			}
		}
		break;

		case eBytecodeChunkType::EVENT:
		{
			switch ( type )
			{
				case eBytecodeChunkType::STATE:
				{
					ReportError( "Cannot define a State inside of an OnEvent definition" );
					return false;
				}
				break;

				case eBytecodeChunkType::EVENT:
				{
					ReportError( "Cannot define an Event inside of another OnEvent definition" );
					return false;
				}
				break;
			}
		}
		break;

		default:
		{
			ReportError( "Tried to define a new bytecode chunk while inside an invalid chunk, make Tyler fix this" );
			return false;
		}
		break;
	}

	// This is a valid chunk definition, create with current chunk as parent scope
	ZephyrBytecodeChunk* newChunk = new ZephyrBytecodeChunk( chunkName, m_curBytecodeChunk );
	
	/*for ( auto globalVar : m_curBytecodeChunk->GetVariables() )
	{
		newChunk->SetVariable( globalVar.first, globalVar.second );
	}*/

	newChunk->SetType( type );

	// Save any state chunks into a map for the ZephyrScriptDefinition
	if ( type == eBytecodeChunkType::STATE )
	{
		// Set the first state seen in the file as the initial state
		if ( m_isFirstStateDef )
		{
			m_isFirstStateDef = false;

			newChunk->SetAsInitialState();
		}

		m_bytecodeChunks[chunkName] = newChunk;
	}

	// Save event chunk into parent chunk's event map
	if ( type == eBytecodeChunkType::EVENT )
	{
		m_curBytecodeChunk->AddEventChunk( newChunk );
	}


	m_curBytecodeChunk = newChunk;
	m_curBytecodeChunksStack.push( newChunk );

	return true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::FinalizeCurBytecodeChunk()
{
	m_curBytecodeChunksStack.pop();

	if ( !m_curBytecodeChunksStack.empty() )
	{
		m_curBytecodeChunk = m_curBytecodeChunksStack.top();
	}
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::WriteByteToCurChunk( byte newByte )
{
	if ( m_curBytecodeChunk == nullptr )
	{
		ReportError( "No active bytecode chunks to write data to, make Tyler fix this" );
		return false;
	}

	m_curBytecodeChunk->WriteByte( newByte );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::WriteOpCodeToCurChunk( eOpCode opCode, int lineNum )
{
	if ( m_curBytecodeChunk == nullptr )
	{
		ReportError( "No active bytecode chunks to write data to, make Tyler fix this" );
		return false;
	}

	m_curBytecodeChunk->WriteByte( opCode );

	//AddOpCodeLineNumToMap( m_curBytecodeChunk->GetFullyQualifiedName(), (int)opCode );
	AddOpCodeLineNumToMap( m_curBytecodeChunk->GetFullyQualifiedName(), lineNum );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::WriteConstantOpToCurChunk( const ZephyrValue& constant, int lineNum )
{
	if ( !WriteConstantToCurChunk( constant ) )
	{
		return false;
	}

	//AddOpCodeLineNumToMap( m_curBytecodeChunk->GetFullyQualifiedName(), 3 );
	AddOpCodeLineNumToMap( m_curBytecodeChunk->GetFullyQualifiedName(), lineNum );
	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::WriteConstantToCurChunk( const ZephyrValue& constant )
{
	if ( m_curBytecodeChunk == nullptr )
	{
		ReportError( "No active bytecode chunks to write data to, make Tyler fix this" );
		return false;
	}

	m_curBytecodeChunk->WriteByte( eOpCode::CONSTANT );
	m_curBytecodeChunk->WriteConstant( constant );

	return true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::AddOpCodeLineNumToMap( const std::string& bytecodeChunkName, int lineNum )
{
	const auto& mapIter = m_bytecodeOpCodeToLineNumMap.find( bytecodeChunkName );
	if ( mapIter == m_bytecodeOpCodeToLineNumMap.cend() )
	{
		m_bytecodeOpCodeToLineNumMap[bytecodeChunkName] = std::vector<int>();
	}

	m_bytecodeOpCodeToLineNumMap[bytecodeChunkName].push_back( lineNum );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseBlock()
{
	// Parse first brace
	if ( !ConsumeExpectedNextToken( eTokenType::BRACE_LEFT ) )
	{
		return false;
	}

	ZephyrToken curToken = GetCurToken();

	// Parse statements in block
	while ( !DoesTokenMatchType( curToken, eTokenType::BRACE_RIGHT )
			&& !DoesTokenMatchType( curToken, eTokenType::END_OF_FILE ) )
	{
		if ( !ParseStatement() )
		{
			return false;
		}

		curToken = GetCurToken();
	}

	// Check for closing brace
	if ( !ConsumeExpectedNextToken( eTokenType::BRACE_RIGHT ) )
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseStatement()
{
	ZephyrToken curToken = ConsumeCurToken();

	// Error if this statement is invalid for chunk type
	if ( !IsStatementValidForChunk( curToken.GetType(), m_curBytecodeChunk->GetType() ) )
	{
		return false;
	}

	switch ( curToken.GetType() )
	{
		case eTokenType::STATE:				
		{
			return ParseStateDefinition();
		}
		break;

		case eTokenType::FUNCTION:
		{
			return ParseFunctionDefinition();
		}
		
		case eTokenType::ENTITY:
		{
			if ( !ParseVariableDeclaration( eValueType::ENTITY ) )
			{
				return false;
			}
		}
		break;
		
		case eTokenType::USER_TYPE:
		{
			if ( !ParseVariableDeclaration( eValueType::USER_TYPE, curToken.GetData() ) )
			{
				return false;
			}

		}
		break;

		case eTokenType::ON_ENTER:
		case eTokenType::ON_EXIT:
		case eTokenType::ON_UPDATE:
		{
			if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) ) { return false; }
			if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT ) ) {	return false; }

			bool succeeded = CreateBytecodeChunk( ToString( curToken.GetType() ), eBytecodeChunkType::EVENT );
			if ( !succeeded )
			{
				return false;
			}

			succeeded = ParseBlock();

			FinalizeCurBytecodeChunk();

			return succeeded;
		}
		break;
		
		case eTokenType::CHANGE_STATE:
		{
			if ( !ParseChangeStateStatement() )
			{
				return false;
			}
		}
		break;

		case eTokenType::IF:
		{
			return ParseIfStatement();
		}
		break;

		case eTokenType::RETURN:
		{
			WriteOpCodeToCurChunk( eOpCode::RETURN, curToken.GetLineNum() );
		}
		break;
		
		case eTokenType::IDENTIFIER:
		{
			// Check if this is a function name by looking for opening paren
			if ( GetCurTokenType() == eTokenType::PARENTHESIS_LEFT  )
			{
				if ( !ParseFunctionCall() )
				{
					return false;
				}
				
			}
			else if ( GetCurTokenType() == eTokenType::PERIOD )
			{
				BackupToLastToken();
				return ParseMemberAccessor();
			}
			else
			{
				if ( !ParseAssignment() )
				{
					return false;
				}
			}
		}
		break;

		// End of block
		case eTokenType::BRACE_RIGHT:
		{
			return true;
		}
		break;

		default:
		{
			std::string errorMsg( "Unknown statement '" );
			errorMsg += curToken.GetDebugName();
			errorMsg += "' seen";

			ReportError( errorMsg );
			return false;
		}
		break;
	}

	AdvanceThroughAllMatchingTokens( eTokenType::SEMICOLON );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseStateDefinition()
{
	ZephyrToken curToken = ConsumeCurToken();
	if ( curToken.GetType() != eTokenType::IDENTIFIER )
	{
		ReportError( "State must be followed by a name" );
		return false;
	}

	bool succeeded = CreateBytecodeChunk( curToken.GetData(), eBytecodeChunkType::STATE );
	if ( !succeeded )
	{
		return false;
	}

	succeeded = ParseBlock();

	FinalizeCurBytecodeChunk();

	return succeeded;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseFunctionDefinition()
{
	ZephyrToken functionNameToken = ConsumeCurToken();
	if ( functionNameToken.GetType() != eTokenType::IDENTIFIER )
	{
		ReportError( "Function must be specified in the form: Function Example()" );
		return false;
	}

	if ( g_zephyrAPI->IsMethodRegistered( functionNameToken.GetData() ) )
	{
		ReportError( Stringf( "Function '%s' is already defined as a reserved event in GameEvents and can't be redefined here", functionNameToken.GetData().c_str() ) );
		return false;
	}

	// Create this function chunk so params are saved inside
	bool succeeded = CreateBytecodeChunk( functionNameToken.GetData(), eBytecodeChunkType::EVENT );
	if ( !succeeded )
	{
		return false;
	}

	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) )
	{
		return false;
	}

	ZephyrToken curToken = ConsumeCurToken();
	while ( curToken.GetType() != eTokenType::PARENTHESIS_RIGHT )
	{
		switch ( curToken.GetType() )
		{
			case eTokenType::ENTITY:	if ( !ParseVariableDeclaration( eValueType::ENTITY ) )							{ return false; } break;
			case eTokenType::USER_TYPE: if ( !ParseVariableDeclaration( eValueType::USER_TYPE, curToken.GetData() ) )	{ return false; } break;

			default:
			{
				ReportError( Stringf( "Only variables can be declared inside Function() definition, found %s", ToString( curToken.GetType() ).c_str() ) );
				return false;
			}
		}

		AdvanceToNextTokenIfTypeMatches( eTokenType::COMMA );
		curToken = ConsumeCurToken();
	}

	succeeded = ParseBlock();

	FinalizeCurBytecodeChunk();

	return succeeded;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseVariableDeclaration( const eValueType& varType, const std::string& typeName )
{
	ZephyrToken identifier = ConsumeCurToken();
	if ( !DoesTokenMatchType( identifier, eTokenType::IDENTIFIER ) )
	{
		ReportError( Stringf( "Expected variable name after '%s'", ToString( varType ).c_str() ) );
		return false;
	}

	if ( !DeclareVariable( identifier, varType, typeName ) )
	{
		return false;
	}

	// Check for assignment in line
	ZephyrToken curToken = GetCurToken();
	switch ( curToken.GetType() )
	{
		case eTokenType::EQUAL:
		{
			AdvanceToNextToken();

			if ( !ParseExpression() )
			{
				return false;
			}

			WriteConstantOpToCurChunk( ZephyrValue( identifier.GetData() ), curToken.GetLineNum() );
			WriteOpCodeToCurChunk( eOpCode::ASSIGNMENT, curToken.GetLineNum() );
		}
		break;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseFunctionCall()
{
	ZephyrToken functionName = GetLastToken();
	
	// Function call opening paren
	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) )
	{
		return false;
	}

	if ( !ParseEventArgs() )
	{
		return false;
	}
	
	// We should be one token past the closing paren
	if ( GetLastToken().GetType() != eTokenType::PARENTHESIS_RIGHT )
	{
		ReportError( "Expected ')' after parameter list for function call" );
		return false;
	}

	WriteConstantOpToCurChunk( ZephyrValue( functionName.GetData() ), functionName.GetLineNum() );
	WriteOpCodeToCurChunk( eOpCode::FUNCTION_CALL, functionName.GetLineNum() );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseEventArgs()
{
	ZephyrToken identifier = ConsumeCurToken();
	int paramCount = 0;

	std::vector<std::string> identifierNames;
	std::vector<std::string> identifierParamNames;

	while ( identifier.GetType() == eTokenType::IDENTIFIER )
	{
		if ( !ConsumeExpectedNextToken( eTokenType::COLON ) )
		{
			ReportError( "Parameters must be in the form, var: value" );
			return false;
		}

		ZephyrToken valueToken = GetCurToken();
		switch ( valueToken.GetType() )
		{
			case eTokenType::CONSTANT_NUMBER:
			case eTokenType::ENTITY:
			case eTokenType::TRUE_TOKEN:
			case eTokenType::FALSE_TOKEN:
			case eTokenType::NULL_TOKEN:
			case eTokenType::CONSTANT_STRING:
			{
				if ( !ParseExpression() )
				{
					return false;
				}
			}
			break;
			
			case eTokenType::IDENTIFIER:
			{
				// Only pass single identifiers by reference
				eTokenType nextType = PeekNextToken().GetType();
				if ( nextType == eTokenType::COMMA
					 || nextType == eTokenType::PARENTHESIS_RIGHT 
					 || nextType == eTokenType::IDENTIFIER )
				{
					identifierNames.push_back( valueToken.GetData() );
					identifierParamNames.push_back( identifier.GetData() );
				}

				if ( !ParseExpression() )
				{
					return false;
				}
			}
			break;

			default:
			{
				ReportError( "Must set parameter equal to a value in the form, var: value" );
				return false;
			}
		}

		WriteConstantOpToCurChunk( identifier.GetData(), valueToken.GetLineNum() );
		++paramCount;

		AdvanceToNextTokenIfTypeMatches( eTokenType::COMMA );

		identifier = ConsumeCurToken();
	}

	WriteConstantOpToCurChunk( ZephyrValue( (float)paramCount ), identifier.GetLineNum() );

	for ( int identifierIdx = 0; identifierIdx < (int)identifierParamNames.size(); ++identifierIdx )
	{
		WriteConstantOpToCurChunk( identifierNames[identifierIdx], identifier.GetLineNum() );
		WriteConstantOpToCurChunk( identifierParamNames[identifierIdx], identifier.GetLineNum() );
	}

	WriteConstantOpToCurChunk( ZephyrValue( (float)identifierParamNames.size() ), identifier.GetLineNum() );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseParameters()
{
	ZephyrToken identifier = ConsumeCurToken();
	int paramCount = 0;

	while ( identifier.GetType() == eTokenType::IDENTIFIER )
	{
		if ( !ConsumeExpectedNextToken( eTokenType::COLON ) )
		{
			ReportError( "Parameters must be in the form, var: value" );
			return false;
		}

		ZephyrToken valueToken = GetCurToken();
		switch ( valueToken.GetType() )
		{
			case eTokenType::MINUS:
			case eTokenType::CONSTANT_NUMBER:
			case eTokenType::ENTITY:
			case eTokenType::TRUE_TOKEN:
			case eTokenType::FALSE_TOKEN:
			case eTokenType::NULL_TOKEN:
			case eTokenType::CONSTANT_STRING:
			case eTokenType::USER_TYPE:
			case eTokenType::IDENTIFIER:
			{
				if ( !ParseExpression() )
				{
					return false;
				}
			}
			break;

			default:
			{
				ReportError( "Must set parameter equal to a value in the form, var: value" );
				return false;
			}
		}

		WriteConstantOpToCurChunk( identifier.GetData(), identifier.GetLineNum() );
		++paramCount;

		AdvanceToNextTokenIfTypeMatches( eTokenType::COMMA );

		identifier = ConsumeCurToken();
	}

	WriteConstantOpToCurChunk( ZephyrValue( (float)paramCount ), identifier.GetLineNum() );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseChangeStateStatement()
{
	// Opening paren
	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) )
	{
		return false;
	}

	ZephyrToken stateName = ConsumeCurToken();
	if ( !DoesTokenMatchType( stateName, eTokenType::IDENTIFIER ) )
	{
		ReportError( "ChangeState must specify a target state" );
		return false;
	}

	// TODO: Check that the state name is valid


	// Closing paren
	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT ) )
	{
		return false;
	}

	WriteConstantOpToCurChunk( ZephyrValue( stateName.GetData() ), stateName.GetLineNum() );
	WriteOpCodeToCurChunk( eOpCode::CHANGE_STATE, stateName.GetLineNum() );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseIfStatement()
{
	// Write a placeholder for how many bytes the if block is so we can update it with the length later
	m_curBytecodeChunk->WriteByte( eOpCode::CONSTANT );
	int ifInstructionCountIdx = m_curBytecodeChunk->AddConstant( ZephyrValue( 0.f ) );
	m_curBytecodeChunk->WriteByte( ifInstructionCountIdx );
	// Manually add the constant op code to the lineNum map
	AddOpCodeLineNumToMap( m_curBytecodeChunk->GetFullyQualifiedName(), GetCurToken().GetLineNum() );

	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) ) return false;

	if ( !ParseExpression() ) return false;

	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT ) ) return false;

	// Need the last token to use the closing parenthesis line for error reporting
	int lastLineNum = GetLastToken().GetLineNum();
	WriteOpCodeToCurChunk( eOpCode::IF, lastLineNum );

	int preIfBlockByteCount = (int)m_curBytecodeChunk->GetCode().size();

	if ( !ParseBlock() ) return false;

	// Set the number of bytes to jump to be the size of the if block plus 3 bytes
	// 2 for the constant declaration and 1 for the JUMP op to jump over the else statement
	m_curBytecodeChunk->SetConstantAtIdx( ifInstructionCountIdx, ZephyrValue( (float)( m_curBytecodeChunk->GetCode().size() - preIfBlockByteCount ) + 3.f ) );

	// Write a placeholder for the jump over the else block
	m_curBytecodeChunk->WriteByte( eOpCode::CONSTANT );
	int elseInstructionCountIdx = m_curBytecodeChunk->AddConstant( ZephyrValue( 0.f ) );
	m_curBytecodeChunk->WriteByte( elseInstructionCountIdx );
	// Manually add the constant op code to the lineNum map
	AddOpCodeLineNumToMap( m_curBytecodeChunk->GetFullyQualifiedName(), lastLineNum );

	WriteOpCodeToCurChunk( eOpCode::JUMP, lastLineNum );

	// Check for else statement
	if ( GetCurToken().GetType() == eTokenType::ELSE )
	{
		AdvanceToNextToken();

		int preElseBlockByteCount = (int)m_curBytecodeChunk->GetCode().size();

		if ( !ParseBlock() ) return false;

		m_curBytecodeChunk->SetConstantAtIdx( elseInstructionCountIdx, ZephyrValue( (float)( m_curBytecodeChunk->GetCode().size() - preElseBlockByteCount ) ) );
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseAssignment()
{
	// Start at the identifier for TryToGetVariable to work properly
	BackupToLastToken();
	ZephyrToken identifier = GetCurToken();
	ZephyrToken nextToken = PeekNextToken();

	ZephyrValue value;
	switch ( nextToken.GetType() )
	{
		case eTokenType::EQUAL:
		{
			// Advance to the actual value for expression
			AdvanceToNextToken();
			AdvanceToNextToken();

			if ( !ParseExpression() )
			{
				return false;
			}

			WriteConstantOpToCurChunk( ZephyrValue( identifier.GetData() ), identifier.GetLineNum() );
			WriteOpCodeToCurChunk( eOpCode::ASSIGNMENT, identifier.GetLineNum() );
		}
		break;

		default:
		{
			ReportError( Stringf( "Assignment to variable '%s' expected a '=' sign after the variable name", identifier.GetData().c_str() ) );
			return false;
		}
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseMemberAssignment()
{
	BackupToLastToken();
	ZephyrToken identifier = GetCurToken();
	ZephyrToken nextToken = PeekNextToken();

	eValueType valType;
	if ( !TryToGetVariableType( identifier.GetData(), valType ) )
	{
		ReportError( Stringf( "Cannot assign to an undefined variable, '%s'", identifier.GetData().c_str() ) );
		return false;
	}
	
	AdvanceToNextToken();
	AdvanceToNextToken();
	ZephyrToken member = ConsumeCurToken();

	if ( !ConsumeExpectedNextToken( eTokenType::EQUAL ) )
	{
		ReportError( Stringf( "Assignment to variable '%s.%s' expected a '=' sign after the variable name", identifier.GetData().c_str(), member.GetData().c_str() ) );
		return false;
	}

	if ( !ParseExpression() )
	{
		return false;
	}

	WriteConstantOpToCurChunk( ZephyrValue( identifier.GetData() ), identifier.GetLineNum() );
	WriteConstantOpToCurChunk( ZephyrValue( member.GetData() ), member.GetLineNum() );
	WriteOpCodeToCurChunk( eOpCode::MEMBER_ASSIGNMENT, member.GetLineNum() );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseMemberAccessor()
{
	ZephyrToken topLevelObj = GetCurToken();
	
	// Advance to first period
	AdvanceToNextToken();
	
	int memberCount = 0;
	while ( GetCurTokenType() == eTokenType::PERIOD )
	{
		++memberCount;

		// Advance past period
		AdvanceToNextToken();

		ZephyrToken member = ConsumeCurToken();

		if ( member.GetType() != eTokenType::IDENTIFIER )
		{
			ReportError( Stringf( "Invalid symbol seen after '.': '%s' Only variable or function names can follow '.'", member.GetData().c_str() ) );
			return false;
		}

		WriteConstantOpToCurChunk( ZephyrValue( member.GetData() ), member.GetLineNum() );
	}

	// Write number of accessors as number
	WriteConstantOpToCurChunk( ZephyrValue( topLevelObj.GetData() ), topLevelObj.GetLineNum() );
	WriteConstantOpToCurChunk( ZephyrValue( (float)memberCount ), topLevelObj.GetLineNum() );
		
	switch ( GetCurTokenType() )
	{
		case eTokenType::EQUAL:
		{
			AdvanceToNextToken();
			if ( !ParseExpression() )
			{
				return false;
			}

			WriteOpCodeToCurChunk( eOpCode::MEMBER_ASSIGNMENT, GetCurToken().GetLineNum() );
		}
		break;

		case eTokenType::PARENTHESIS_LEFT:
		{
			AdvanceToNextToken();
			if ( !ParseEventArgs() )
			{
				return false;
			}

			// We should be one token past the closing paren
			if ( GetLastToken().GetType() != eTokenType::PARENTHESIS_RIGHT )
			{
				ReportError( "Expected ')' after parameter list for function call" );
				return false;
			}

			WriteOpCodeToCurChunk( eOpCode::MEMBER_FUNCTION_CALL, GetLastToken().GetLineNum() );
		}
		break;

		// If something else is at current token, must just be an accessor in an expression
		default:
		{
			WriteOpCodeToCurChunk( eOpCode::MEMBER_ACCESSOR, GetLastToken().GetLineNum() );
		}
		break;
	}
	
	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseExpression()
{
	return ParseExpressionWithPrecedenceLevel( eOpPrecedenceLevel::ASSIGNMENT );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseExpressionWithPrecedenceLevel( eOpPrecedenceLevel precLevel )
{
	ZephyrToken curToken = GetCurToken();
	if ( !CallPrefixFunction( curToken ) )
	{
		return false;
	}

	curToken = GetCurToken();
	while ( precLevel <= GetPrecedenceLevel( curToken ) )
	{
		if ( !CallInfixFunction( curToken ) )
		{
			return false;
		}

		curToken = GetCurToken();
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::CallPrefixFunction( const ZephyrToken& token )
{
	switch ( token.GetType() )
	{
		case eTokenType::PARENTHESIS_LEFT:	return ParseParenthesesGroup();
		case eTokenType::MINUS:				return ParseUnaryExpression();
		case eTokenType::BANG:				return ParseUnaryExpression();

		case eTokenType::CONSTANT_NUMBER:	return ParseNumberConstant();
		case eTokenType::TRUE_TOKEN:		return ParseBoolConstant( true );
		case eTokenType::FALSE_TOKEN:		return ParseBoolConstant( false );
		case eTokenType::NULL_TOKEN:		return ParseEntityConstant();
		case eTokenType::CONSTANT_STRING:	return ParseStringConstant();
		case eTokenType::USER_TYPE:			return ParseUserTypeConstant( token.GetData() );

		case eTokenType::ENTITY:
		case eTokenType::IDENTIFIER:
		{
			if ( PeekNextToken().GetType() == eTokenType::EQUAL )
			{
				// TODO: advance just to back up?
				AdvanceToNextToken();
				return ParseAssignment();
			}

			// Need to keep checking for = sign here
			if ( PeekNextToken().GetType() == eTokenType::PERIOD )
			{
				return ParseMemberAccessor();
			}

			return ParseIdentifierExpression();
		}
		break;
	}

	// TODO: Make this more descriptive
	ReportError( "Missing expression" );
	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::CallInfixFunction( const ZephyrToken& token )
{
	switch ( token.GetType() )
	{
		case eTokenType::PLUS:
		case eTokenType::MINUS:
		case eTokenType::STAR:
		case eTokenType::SLASH:
		case eTokenType::BANG_EQUAL:
		case eTokenType::EQUAL_EQUAL:
		case eTokenType::GREATER:
		case eTokenType::GREATER_EQUAL:
		case eTokenType::LESS:
		case eTokenType::LESS_EQUAL:
		case eTokenType::AND:
		case eTokenType::OR:
		{
			return ParseBinaryExpression();
		}
		break;
	}

	// TODO: Make this more descriptive
	ReportError( "Missing expression" );
	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseParenthesesGroup()
{
	ConsumeCurToken();

	if ( !ParseExpression() )
	{
		return false;
	}
	
	return ConsumeExpectedNextToken( eTokenType::PARENTHESIS_RIGHT );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseUnaryExpression()
{
	ZephyrToken curToken = ConsumeCurToken();

	if ( !ParseExpressionWithPrecedenceLevel( eOpPrecedenceLevel::UNARY ) )
	{
		return false;
	}

	switch ( curToken.GetType() )
	{
		case eTokenType::MINUS:			return WriteOpCodeToCurChunk( eOpCode::NEGATE, curToken.GetLineNum() );
		case eTokenType::BANG:			return WriteOpCodeToCurChunk( eOpCode::NOT, curToken.GetLineNum() );
	}
		
	ReportError( Stringf( "Invalid unary operation '%s'", ToString( curToken.GetType() ).c_str() ) );
	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseBinaryExpression()
{
	ZephyrToken curToken = ConsumeCurToken();

	eOpPrecedenceLevel precLevel = GetNextHighestPrecedenceLevel( curToken );

	if ( !ParseExpressionWithPrecedenceLevel( precLevel ) )
	{
		return false;
	}

	switch ( curToken.GetType() )
	{
		case eTokenType::PLUS:			return WriteOpCodeToCurChunk( eOpCode::ADD,				curToken.GetLineNum() );
		case eTokenType::MINUS:			return WriteOpCodeToCurChunk( eOpCode::SUBTRACT,		curToken.GetLineNum() );
		case eTokenType::STAR:			return WriteOpCodeToCurChunk( eOpCode::MULTIPLY,		curToken.GetLineNum() );
		case eTokenType::SLASH:			return WriteOpCodeToCurChunk( eOpCode::DIVIDE,			curToken.GetLineNum() );
		case eTokenType::BANG_EQUAL:	return WriteOpCodeToCurChunk( eOpCode::NOT_EQUAL,		curToken.GetLineNum() );
		case eTokenType::EQUAL_EQUAL:	return WriteOpCodeToCurChunk( eOpCode::EQUAL,			curToken.GetLineNum() );
		case eTokenType::GREATER:		return WriteOpCodeToCurChunk( eOpCode::GREATER,			curToken.GetLineNum() );
		case eTokenType::GREATER_EQUAL:	return WriteOpCodeToCurChunk( eOpCode::GREATER_EQUAL,	curToken.GetLineNum() );
		case eTokenType::LESS:			return WriteOpCodeToCurChunk( eOpCode::LESS,			curToken.GetLineNum() );
		case eTokenType::LESS_EQUAL:	return WriteOpCodeToCurChunk( eOpCode::LESS_EQUAL,		curToken.GetLineNum() );
		case eTokenType::AND:			return WriteOpCodeToCurChunk( eOpCode::AND,				curToken.GetLineNum() );
		case eTokenType::OR:			return WriteOpCodeToCurChunk( eOpCode::OR,				curToken.GetLineNum() );
	}

	ReportError( Stringf( "Invalid binary operation '%s'", ToString( curToken.GetType() ).c_str() ) );
	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseNumberConstant()
{
	ZephyrToken curToken = ConsumeCurToken();

	ZephyrArgs params;
	params.SetValue( "value", (NUMBER_TYPE)atof( curToken.GetData().c_str() ) );
	ZephyrValue numberConstant = ZephyrValue( g_zephyrTypeHandleFactory->CreateHandle( ZephyrEngineTypeNames::NUMBER, &params ) );

	return WriteConstantOpToCurChunk( numberConstant, curToken.GetLineNum() );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseBoolConstant( bool value )
{
	ZephyrToken curToken = ConsumeCurToken();

	ZephyrArgs params;
	params.SetValue( "value", value );
	ZephyrValue boolConstant = ZephyrValue( g_zephyrTypeHandleFactory->CreateHandle( ZephyrEngineTypeNames::BOOL, &params ) );

	return WriteConstantOpToCurChunk( boolConstant, curToken.GetLineNum() );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseEntityConstant()
{
	ZephyrToken curToken = ConsumeCurToken();

	return WriteConstantOpToCurChunk( ZephyrValue( INVALID_ENTITY_ID ), curToken.GetLineNum() );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseStringConstant()
{
	ZephyrToken curToken = ConsumeCurToken();

	ZephyrArgs params;
	params.SetValue( "value", curToken.GetData() );
	ZephyrValue stringConstant( g_zephyrTypeHandleFactory->CreateHandle( ZephyrEngineTypeNames::STRING, &params ) );

	return WriteConstantOpToCurChunk( stringConstant, curToken.GetLineNum() );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseUserTypeConstant( const std::string& typeName )
{
	AdvanceToNextToken();

	// Constructor opening paren
	if ( !ConsumeExpectedNextToken( eTokenType::PARENTHESIS_LEFT ) )
	{
		return false;
	}

	if ( !ParseParameters() )
	{
		return false;
	}

	// We should be one token past the closing paren
	if ( GetLastToken().GetType() != eTokenType::PARENTHESIS_RIGHT )
	{
		ReportError( "Expected ')' after parameter list for object constructor" );
		return false;
	}

	int lastTokenLineNum = GetLastToken().GetLineNum();
	WriteConstantOpToCurChunk( ZephyrValue( typeName ), lastTokenLineNum );
	WriteConstantOpToCurChunk( ZephyrValue( "TEMPORARY_VAR" ), lastTokenLineNum );
	WriteOpCodeToCurChunk( eOpCode::CONSTANT_USER_TYPE, lastTokenLineNum );

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ParseIdentifierExpression()
{
	ZephyrToken curToken = ConsumeCurToken();

	if ( curToken.GetType() != eTokenType::IDENTIFIER )
	{
		ReportError( Stringf( "Expected Identifier, but found '%s'", ToString( curToken.GetType() ).c_str() ) );
		return false;
	}

	WriteConstantOpToCurChunk( ZephyrValue( curToken.GetData() ), curToken.GetLineNum() );
	WriteOpCodeToCurChunk( eOpCode::GET_VARIABLE_VALUE, curToken.GetLineNum() );

	return true;
}


//-----------------------------------------------------------------------------------------------
eOpPrecedenceLevel ZephyrParser::GetPrecedenceLevel( const ZephyrToken& token )
{
	switch ( token.GetType() )
	{
		case eTokenType::PLUS:				return eOpPrecedenceLevel::TERM;
		case eTokenType::MINUS:				return eOpPrecedenceLevel::TERM;
		case eTokenType::STAR:				return eOpPrecedenceLevel::FACTOR;
		case eTokenType::SLASH:				return eOpPrecedenceLevel::FACTOR;
		case eTokenType::BANG_EQUAL:		return eOpPrecedenceLevel::EQUALITY;
		case eTokenType::EQUAL_EQUAL:		return eOpPrecedenceLevel::EQUALITY;
		case eTokenType::GREATER:			return eOpPrecedenceLevel::COMPARISON;
		case eTokenType::GREATER_EQUAL:		return eOpPrecedenceLevel::COMPARISON;
		case eTokenType::LESS:				return eOpPrecedenceLevel::COMPARISON;
		case eTokenType::LESS_EQUAL:		return eOpPrecedenceLevel::COMPARISON;
		case eTokenType::AND:				return eOpPrecedenceLevel::AND;
		case eTokenType::OR:				return eOpPrecedenceLevel::OR;
		case eTokenType::PERIOD:			return eOpPrecedenceLevel::CALL;
		case eTokenType::PARENTHESIS_LEFT:	return eOpPrecedenceLevel::CALL;
		case eTokenType::EQUAL:				return eOpPrecedenceLevel::ASSIGNMENT;
		default:							return eOpPrecedenceLevel::NONE;
	}
}


//-----------------------------------------------------------------------------------------------
eOpPrecedenceLevel ZephyrParser::GetNextHighestPrecedenceLevel( const ZephyrToken& token )
{
	eOpPrecedenceLevel precLevel = GetPrecedenceLevel( token );

	//TODO: Make this safer
	return (eOpPrecedenceLevel)( (int)precLevel + 1 );
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::ReportError( const std::string& errorMsg )
{
	ZephyrToken const& curToken = GetLastToken();

	std::string fullErrorStr = Stringf( "Error - %s: line %i: %s", m_filename.c_str(), curToken.GetLineNum(), errorMsg.c_str() );

	g_devConsole->PrintError( fullErrorStr );
}


//-----------------------------------------------------------------------------------------------
ZephyrToken ZephyrParser::PeekNextToken() const
{
	if ( IsAtEnd() )
	{
		return m_tokens[m_tokens.size() - 1];
	}

	return m_tokens[m_curTokenIdx + 1];
}


//-----------------------------------------------------------------------------------------------
ZephyrToken ZephyrParser::PeekNextNextToken() const
{
	if ( IsAtEnd() 
		 || ( m_curTokenIdx + 2 ) > (int)m_tokens.size() - 1 )
	{
		return m_tokens[m_tokens.size() - 1];
	}

	return m_tokens[m_curTokenIdx + 2];
}


//-----------------------------------------------------------------------------------------------
ZephyrToken ZephyrParser::ConsumeCurToken()
{
	if ( IsAtEnd() )
	{
		return m_tokens[m_tokens.size() - 1] ;
	}

	int tokenIdx = m_curTokenIdx;
	++m_curTokenIdx;

	if ( !IsAtEnd()
		 && m_tokens[m_curTokenIdx].GetType() == eTokenType::ERROR_TOKEN )
	{
		ReportError( m_tokens[m_curTokenIdx].GetData() );
	}

	return m_tokens[tokenIdx];
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::AdvanceToNextToken()
{
	if ( IsAtEnd() )
	{
		 return;
	}

	++m_curTokenIdx;

	if ( !IsAtEnd()
		 && m_tokens[m_curTokenIdx].GetType() == eTokenType::ERROR_TOKEN )
	{
		ReportError( m_tokens[m_curTokenIdx].GetData() );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::BackupToLastToken()
{
	--m_curTokenIdx;
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::AdvanceToNextTokenIfTypeMatches( eTokenType expectedType )
{
	if ( GetCurToken().GetType() == expectedType )
	{
		AdvanceToNextToken();
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrParser::AdvanceThroughAllMatchingTokens( eTokenType expectedType )
{
	while ( GetCurToken().GetType() == expectedType )
	{
		AdvanceToNextToken();
	}
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::ConsumeExpectedNextToken( eTokenType expectedType )
{
	ZephyrToken curToken = ConsumeCurToken();

	if ( !DoesTokenMatchType( curToken, expectedType ) )
	{
		ReportError( Stringf( "Expected '%s'", ToString( expectedType ).c_str() ) );
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::IsStatementValidForChunk( eTokenType statementToken, eBytecodeChunkType chunkType )
{
	switch ( statementToken )
	{
		// Valid to define anywhere
		case eTokenType::STATE:
		case eTokenType::FUNCTION:
		case eTokenType::ENTITY:
		case eTokenType::USER_TYPE:
		case eTokenType::BRACE_RIGHT:
		{
			return true;
		}
		break;
		
		// Valid to define in state only
		case eTokenType::ON_ENTER:
		case eTokenType::ON_EXIT:
		case eTokenType::ON_UPDATE:
		{
			if ( chunkType != eBytecodeChunkType::STATE )
			{
				ReportError( Stringf( "'%s' can only be defined inside a State", ToString( statementToken ).c_str() ) );
				return false;
			}

			return true;
		}
		break;

		// Valid to define in event only
		case eTokenType::CHANGE_STATE:
		case eTokenType::IF:
		case eTokenType::RETURN:
		case eTokenType::IDENTIFIER:
		{
			if ( chunkType != eBytecodeChunkType::EVENT )
			{
				ReportError( Stringf( "'%s' can only be defined inside an Event", ToString( statementToken ).c_str() ) );
				return false;
			}

			return true;
		}
		break;

		default:
		{
			ReportError( Stringf( "Unknown statement starting with '%s' seen", ToString( statementToken ).c_str() ) );
			return false;
		}
	}
}


//-----------------------------------------------------------------------------------------------
ZephyrToken ZephyrParser::GetLastToken() const
{
	return m_tokens[m_curTokenIdx - 1];
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::IsCurTokenType( const eTokenType& type )
{
	if ( IsAtEnd()
		 || GetCurTokenType() != type )
	{
		return false;
	}

	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::DoesTokenMatchType( const ZephyrToken& token, const eTokenType& type )
{
	if ( token.GetType() != type )
	{
		return false;
	}

	return true;

}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::DeclareVariable( const ZephyrToken& identifier, const eValueType& varType, const std::string& typeName )
{
	return DeclareVariable( identifier.GetData(), varType, typeName );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::DeclareVariable( const std::string& identifier, const eValueType& varType, const std::string& typeName )
{
	// m_curBytecodeChunk will be the global state machine if outside a state declaration, should never be null
	ZephyrValue value;
	switch ( varType )
	{
		case eValueType::ENTITY: 
		{
			// Cannot redefine parent entity
			if ( identifier == PARENT_ENTITY_STR )
			{
				ReportError( Stringf( "Cannot define an entity named '%s'. That name is reserved to reference the parent entity for this script.", PARENT_ENTITY_STR.c_str() ) );
				return false;
			}

			value = ZephyrValue( INVALID_ENTITY_ID ); break;
		}
		case eValueType::USER_TYPE:
		{
			// Declare only, no parameters to invoke default constructor
			value = ZephyrValue( g_zephyrTypeHandleFactory->CreateHandle( typeName, nullptr ) ); break;
		}
	}

	m_curBytecodeChunk->SetVariable( identifier, value );
	return true;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::TryToGetVariable( const std::string& identifier, ZephyrValue& out_value )
{
	bool foundValue = m_curBytecodeChunk->TryToGetVariable( identifier, out_value );

	// TODO: Check parent chunk instead of only global
	if ( !foundValue
		 && m_curBytecodeChunk != m_globalBytecodeChunk )
	{
		foundValue = m_globalBytecodeChunk->TryToGetVariable( identifier, out_value );
	}

	return foundValue;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::TryToGetVariableType( const std::string& identifier, eValueType& out_varType )
{
	ZephyrValue val;
	bool foundValue = m_curBytecodeChunk->TryToGetVariable( identifier, val );

	if ( !foundValue
		 && m_curBytecodeChunk != m_globalBytecodeChunk )
	{
		foundValue = m_globalBytecodeChunk->TryToGetVariable( identifier, val );
	}

	if ( foundValue )
	{
		out_varType = val.GetType();
	}

	return foundValue;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrParser::IsAtEnd() const
{
	return m_curTokenIdx >= (int)m_tokens.size();
}


//-----------------------------------------------------------------------------------------------
ZephyrToken ZephyrParser::GetCurToken() const
{
	return m_tokens[m_curTokenIdx];
}


//-----------------------------------------------------------------------------------------------
eTokenType ZephyrParser::GetCurTokenType() const
{
	return m_tokens[m_curTokenIdx].GetType();
}


//-----------------------------------------------------------------------------------------------
int ZephyrParser::GetCurTokenLineNum() const
{
	return m_tokens[m_curTokenIdx].GetLineNum();
}
