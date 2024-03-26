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
	ZephyrValue finalMemberVal = NULL_ZEPHYR_HANDLE;
	std::string	baseObjName;
	std::vector<std::string> memberNames;
	std::vector<EntityId> entityIdChain;
};


//-----------------------------------------------------------------------------------------------
enum class EAssignToVariableInMapResult
{
	ERROR,
	NOT_FOUND,
	SUCCESS
};


//-----------------------------------------------------------------------------------------------
class ZephyrVirtualMachine
{
	friend class ZephyrInterpreter;

private:
	ZephyrVirtualMachine( const ZephyrBytecodeChunk& bytecodeChunk,
						  ZephyrValueMap* globalVariables,
						  ZephyrComponent& zephyrComponent,
						  ZephyrArgs* eventArgs = nullptr,
						  ZephyrValueMap* stateVariables = nullptr );
	
	void		InterpretBytecodeChunk();

	void		CopyEventArgVariables( ZephyrArgs* args, ZephyrValueMap& localVariables );

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

	ZephyrValue						GetVariableValue			( const std::string& variableName, const ZephyrValueMap& localVariables );
	void							AssignToVariable			( const std::string& variableName, const ZephyrValue& value, ZephyrValueMap& localVariables );
	EAssignToVariableInMapResult	AssignToVariableInMap		( const std::string& variableName, const ZephyrValue& value, ZephyrValueMap* variableMap );
	
	MemberAccessorResult ProcessResultOfMemberAccessor( const ZephyrValueMap& localVariables );
	
	std::map<std::string, std::string> GetCallerVariableToParamNamesFromParameters( const std::string& eventName );
	void InsertParametersIntoEventArgs( ZephyrArgs& args );
	void UpdateIdentifierParameters( const std::map<std::string, std::string>& identifierParams, const ZephyrArgs& args, ZephyrValueMap& localVariables );
	ZephyrValue GetZephyrValFromEventArgs( const std::string& varName, const EventArgs& args );

	ZephyrValue GetGlobalVariableFromEntity	( EntityId entityId, const std::string& variableName );
	void SetGlobalVariableInEntity			( EntityId entityId, const std::string& variableName, const ZephyrValue& value );
	bool CallMemberFunctionOnEntity			( EntityId entityId, const std::string& functionName, ZephyrArgs* args );
	
	bool CallMemberFunctionOnUserType		( ZephyrHandle userObj, const std::string& functionName, ZephyrArgs* args );

	void ReportError( const std::string& errorMsg );
	bool IsErrorValue( const ZephyrValue& zephyrValue );

private:
	ZephyrComponent&		m_zephyrComponent;
	const ZephyrBytecodeChunk&	m_bytecodeChunk;
	int m_curOpCodeIdx = 0;
	std::stack<ZephyrValue> m_constantStack;
	std::deque<std::string> m_curMemberAccessorNames;

	ZephyrValueMap*			m_globalVariables;
	ZephyrValueMap*			m_stateVariables;
	ZephyrValueMap			m_eventVariablesCopy;
	ZephyrValueMap*			m_eventVariables;
	ZephyrArgs*				m_eventArgs;
};
