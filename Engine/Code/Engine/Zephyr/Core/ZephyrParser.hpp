#pragma once
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <string>
#include <stack>
#include <vector>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
class ZephyrScriptDefinition;
class ZephyrToken;
enum class eBytecodeChunkType;


//-----------------------------------------------------------------------------------------------
enum class eOpPrecedenceLevel
{
	NONE,
	ASSIGNMENT,  // =
	OR,          // or
	AND,         // and
	EQUALITY,    // == !=
	COMPARISON,  // < > <= >=
	TERM,        // + -
	FACTOR,      // * /
	UNARY,       // ! -
	CALL,        // . ()
	PRIMARY
};


//-----------------------------------------------------------------------------------------------
class ZephyrParser
{
	friend class ZephyrCompiler;

private:
	// Private constructor so only ZephyrCompiler can use this class
	ZephyrParser( const std::string& filename, const std::vector<ZephyrToken>& tokens );
	
	ZephyrScriptDefinition* ParseTokensIntoScriptDefinition();

	// Bytecode chunk manipulation
	void CreateGlobalBytecodeChunk();
	bool CreateBytecodeChunk( const std::string& chunkName, const eBytecodeChunkType& type );
	void FinalizeCurBytecodeChunk();

	bool WriteByteToCurChunk( byte newByte );
	bool WriteOpCodeToCurChunk( eOpCode opCode, int lineNum );
	bool WriteConstantOpToCurChunk( const ZephyrValue& constant, int lineNum );
	bool WriteConstantToCurChunk( const ZephyrValue& constant );
	void AddOpCodeLineNumToMap( const std::string& bytecodeChunkName, int lineNum );

	bool IsCurTokenType( const eTokenType& type );
	bool DoesTokenMatchType( const ZephyrToken& token, const eTokenType& type );

	bool DeclareVariable( const ZephyrToken& identifier, const std::string& typeName = "" );
	bool DeclareVariable( const std::string& identifier, const std::string& typeName = "" );
	bool TryToGetVariable( const std::string& identifier, ZephyrValue& out_value );

	bool ParseBlock();
	bool ParseStatement();
	bool ParseStateDefinition();
	bool ParseFunctionDefinition();
	bool ParseVariableDeclaration( const std::string& typeName );
	bool ParseFunctionCall();
	bool ParseEventArgs();
	bool ParseParameters();
	bool ParseChangeStateStatement();
	bool ParseIfStatement();
	bool ParseAssignment();
	bool ParseMemberAssignment();
	bool ParseMemberAccessor();
	bool ParseExpression();
	bool ParseExpressionWithPrecedenceLevel( eOpPrecedenceLevel precLevel );
	bool ParseParenthesesGroup();
	bool ParseUnaryExpression();
	bool ParseBinaryExpression();
	bool ParseNumberConstant();
	bool ParseBoolConstant( bool value );
	bool ParseEntityConstant();
	bool ParseStringConstant();
	bool ParseUserTypeConstant( const std::string& typeName );
	bool ParseIdentifierExpression();

	// Pratt Parser Helpers
	bool CallPrefixFunction( const ZephyrToken& token );
	bool CallInfixFunction( const ZephyrToken& token );
	eOpPrecedenceLevel GetPrecedenceLevel( const ZephyrToken& token );
	eOpPrecedenceLevel GetNextHighestPrecedenceLevel( const ZephyrToken& token );

	void ReportError( const std::string& errorMsg );

	ZephyrToken ConsumeCurToken();
	void AdvanceToNextToken();
	void BackupToLastToken();
	void AdvanceToNextTokenIfTypeMatches( eTokenType expectedType );
	void AdvanceThroughAllMatchingTokens( eTokenType expectedType );
	bool ConsumeExpectedNextToken( eTokenType expectedType );
	bool IsStatementValidForChunk( eTokenType statementToken, eBytecodeChunkType chunkType );

	ZephyrToken PeekNextToken() const;
	ZephyrToken PeekNextNextToken() const;
	ZephyrToken GetCurToken() const;
	ZephyrToken GetLastToken() const;
	eTokenType GetCurTokenType() const;
	int GetCurTokenLineNum() const;

	bool IsAtEnd() const;

private:
	std::string m_filename;
	std::vector<ZephyrToken> m_tokens;
	std::map<std::string, std::vector<int>> m_bytecodeOpCodeToLineNumMap;
	int m_curTokenIdx = 0;
	
	bool m_isFirstStateDef = true;
	ZephyrBytecodeChunk* m_globalBytecodeChunk = nullptr;					// Owned by ZephyrScriptDefinition
	std::stack<ZephyrBytecodeChunk*> m_curBytecodeChunksStack;
	ZephyrBytecodeChunkMap m_bytecodeChunks;								// Owned by ZephyrScriptDefinition
	ZephyrBytecodeChunk* m_curBytecodeChunk = nullptr;
};
