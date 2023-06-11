#pragma once
#include "Engine/Zephyr/Core/ZephyrCommon.hpp"

#include <stack>
#include <queue>


//-----------------------------------------------------------------------------------------------
class ZephyrBytecodeChunk;
class ZephyrComponent;


//-----------------------------------------------------------------------------------------------
struct MemberAccessorResult
{
public:
	ZephyrValue finalMemberVal = ZephyrValue::ERROR_VALUE;
	std::string	baseObjName;
	std::vector<std::string> memberNames;
	std::vector<EntityId> entityIdChain;
};


//-----------------------------------------------------------------------------------------------
class ZephyrVirtualMachine
{
	friend class ZephyrInterpreter;

private:
	ZephyrVirtualMachine( ZephyrValueMap* globalVariables,
						  ZephyrComponent& zephyrComponent,
						  EventArgs* eventArgs = nullptr,
						  ZephyrValueMap* stateVariables = nullptr );
	
	void		InterpretBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk );

	void		CopyEventArgVariables( EventArgs* eventArgs, ZephyrValueMap& localVariables );

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

	bool TryToPushVec2MultiplyOp( ZephyrValue& a, ZephyrValue& b );
	bool TryToPushVec3MultiplyOp( ZephyrValue& a, ZephyrValue& b );

	ZephyrValue GetVariableValue			( const std::string& variableName, const ZephyrValueMap& localVariables );
	void		AssignToVariable			( const std::string& variableName, const ZephyrValue& value, ZephyrValueMap& localVariables );
	// ZEPHYR TYPE TODO: Remove/codegen
	void		AssignToVec2MemberVariable	( const std::string& variableName, const std::string& memberName, const ZephyrValue& value, ZephyrValueMap& localVariables );
	void		AssignToVec3MemberVariable	( const std::string& variableName, const std::string& memberName, const ZephyrValue& value, ZephyrValueMap& localVariables );
	
	MemberAccessorResult ProcessResultOfMemberAccessor( const ZephyrValueMap& localVariables );
	
	std::map<std::string, std::string> GetCallerVariableToParamNamesFromParameters( const std::string& eventName );
	void InsertParametersIntoEventArgs( EventArgs& args );
	void UpdateIdentifierParameters( const std::map<std::string, std::string>& identifierParams, const EventArgs& args, ZephyrValueMap& localVariables );
	ZephyrValue GetZephyrValFromEventArgs( const std::string& varName, const EventArgs& args );

	ZephyrValue GetGlobalVariableFromEntity	( EntityId entityId, const std::string& variableName );
	void SetGlobalVariableInEntity			( EntityId entityId, const std::string& variableName, const ZephyrValue& value );
	void SetGlobalVec2MemberVariableInEntity( EntityId entityId, const std::string& variableName, const std::string& memberName, const ZephyrValue& value );
	void SetGlobalVec3MemberVariableInEntity( EntityId entityId, const std::string& variableName, const std::string& memberName, const ZephyrValue& value );
	bool CallMemberFunctionOnEntity			( EntityId entityId, const std::string& functionName, EventArgs* args );

	void ReportError( const std::string& errorMsg );
	bool IsErrorValue( const ZephyrValue& zephyrValue );

private:
	ZephyrComponent&		m_zephyrComponent;
	std::stack<ZephyrValue> m_constantStack;
	std::deque<std::string> m_curMemberAccessorNames;

	ZephyrValueMap*			m_globalVariables;
	ZephyrValueMap*			m_stateVariables;
	ZephyrValueMap			m_eventVariablesCopy;
	ZephyrValueMap*			m_eventVariables;
	EventArgs*				m_eventArgs;
};
