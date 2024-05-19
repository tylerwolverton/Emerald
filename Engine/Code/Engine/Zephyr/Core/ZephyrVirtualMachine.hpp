#pragma once
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Engine/Zephyr/Core/ZephyrBytecodeChunk.hpp"

#include <stack>
#include <queue>


//-----------------------------------------------------------------------------------------------
class ZephyrComponent;


//-----------------------------------------------------------------------------------------------
struct MemberAccessorResult
{
public:
	ZephyrValue finalMemberVal;
	std::string	baseObjName;
	std::vector<std::string> memberNames;
	std::vector<EntityId> entityIdChain;
};


//-----------------------------------------------------------------------------------------------
class ZephyrVirtualMachine
{
	friend class ZephyrInterpreter;

private:
	ZephyrVirtualMachine( ZephyrBytecodeChunk& bytecodeChunk,
						  ZephyrComponent& zephyrComponent,
						  ZephyrArgs* eventArgs = nullptr );
	
	void		InterpretBytecodeChunk();

	void		CopyEventArgsToZephyrVariables( ZephyrArgs* args );
	void		CopyZephyrVariablesToEventArgs( ZephyrArgs* args );

	void		PushConstant( const ZephyrValue& number );
	ZephyrValue PopConstant();
	ZephyrValue PeekConstant();

	void PushBinaryOp		( ZephyrValue& a, ZephyrValue& b, eOpCode opCode );
	void PushAddOp			( ZephyrValue& a, ZephyrValue& b );
	void PushSubtractOp		( ZephyrValue& a, ZephyrValue& b );
	void PushMultiplyOp		( ZephyrValue& a, ZephyrValue& b );
	void PushDivideOp		( ZephyrValue& a, ZephyrValue& b );
	void PushNotEqualOp		( ZephyrValue& a, ZephyrValue& b );
	void PushEqualOp		( ZephyrValue& a, ZephyrValue& b );
	void PushGreaterOp		( ZephyrValue& a, ZephyrValue& b );
	void PushGreaterEqualOp	( ZephyrValue& a, ZephyrValue& b );
	void PushLessOp			( ZephyrValue& a, ZephyrValue& b );
	void PushLessEqualOp	( ZephyrValue& a, ZephyrValue& b );

	ZephyrValue						GetVariableValue			( const std::string& variableName );
	void							AssignToVariable			( const std::string& variableName, const ZephyrValue& value );
	
	MemberAccessorResult ProcessResultOfMemberAccessor();
	
	std::map<std::string, std::string> GetCallerVariableToParamNamesFromParameters( const std::string& eventName );
	void InsertParametersIntoEventArgs( ZephyrArgs& args );
	void UpdateIdentifierParameters( const std::map<std::string, std::string>& identifierParams, const ZephyrArgs& args );
	ZephyrValue GetZephyrValFromEventArgs( const std::string& varName, const ZephyrArgs& args );

	ZephyrValue GetGlobalVariableFromEntity	( EntityId entityId, const std::string& variableName );
	void SetGlobalVariableInEntity			( EntityId entityId, const std::string& variableName, const ZephyrValue& value );
	bool CallMemberFunctionOnEntity			( EntityId entityId, const std::string& functionName, ZephyrArgs* args );
	
	bool CallMemberFunctionOnUserType		( ZephyrHandle userObj, const std::string& functionName, ZephyrArgs* args );

	void ReportError( const std::string& errorMsg );
	bool IsErrorValue( const ZephyrValue& zephyrValue );

private:
	ZephyrComponent&			m_zephyrComponent;
	ZephyrBytecodeChunk&		m_bytecodeChunk;
	int m_curOpCodeIdx = 0;
	std::stack<ZephyrValue>		m_constantStack;

	ZephyrArgs*					m_eventArgs;
};
