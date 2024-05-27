#include "Engine/Zephyr/Core/ZephyrVirtualMachine.hpp"
#include "Engine/Zephyr/Core/ZephyrBytecodeChunk.hpp"
#include "Engine/Zephyr/Core/ZephyrScriptDefinition.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponent.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrEngineEvents.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"
#include "Engine/Zephyr/Types/ZephyrNumber.hpp"
#include "Engine/Zephyr/Types/ZephyrString.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Framework/Entity.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrVirtualMachine::ZephyrVirtualMachine( ZephyrBytecodeChunk& bytecodeChunk,
											ZephyrComponent& zephyrComponent, 
											ZephyrArgs* eventArgs )
	: m_bytecodeChunk( bytecodeChunk )
	, m_zephyrComponent( zephyrComponent )
	, m_eventArgs( eventArgs )
{
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::InterpretBytecodeChunk()
{
	if ( !m_zephyrComponent.IsScriptValid() )
	{
		return;
	}

	m_bytecodeChunk.SetScopeFromIdx( 0 );
	if ( m_bytecodeChunk.GetType() == eBytecodeChunkType::EVENT )
	{
		CopyEventArgsToZephyrVariables( m_eventArgs );
	}

	int curByteIdx = 0;
	// Start at -1 so first op code index is set to 0 when reading next code
	m_curOpCodeIdx = -1;
	while ( curByteIdx < m_bytecodeChunk.GetNumBytes() )
	{
		// If this script has an error during interpretation, bail out to avoid running in a broken, unknown state
		if ( !m_zephyrComponent.IsScriptValid() )
		{
			return;
		}

		++m_curOpCodeIdx;
		byte instruction = m_bytecodeChunk.GetByte( curByteIdx++ );
		eOpCode opCode = ByteToOpCode( instruction );
		switch ( opCode )
		{
			case eOpCode::CONSTANT:
			{
				int constIdx = m_bytecodeChunk.GetByte( curByteIdx++ );
				ZephyrValue constant = m_bytecodeChunk.GetConstant( constIdx );
				PushConstant( constant );
			}
			break;			
			
			case eOpCode::CONSTANT_USER_TYPE:
			{
				ZephyrValue variableName = PopConstant();
				ZephyrValue variableType = PopConstant();

				ZephyrArgs args;
				InsertParametersIntoEventArgs( args );

				PushConstant( ZephyrValue( variableType.ToString(), &args ) );
			}
			break;

			case eOpCode::GET_VARIABLE_VALUE:
			{
				ZephyrValue variableName = PopConstant();
				PushConstant( GetVariableValue( variableName.ToString() ) );
			}
			break;
			
			case eOpCode::ASSIGNMENT:
			{
				ZephyrValue variableName = PopConstant(); 
				ZephyrValue constantValue = PeekConstant();
				AssignToVariable( variableName.ToString(), constantValue );
			}
			break;	

			case eOpCode::MEMBER_ASSIGNMENT:
			{
				ZephyrValue constantValue = PopConstant();

				MemberAccessorResult memberAccessorResult = ProcessResultOfMemberAccessor();

				if ( !memberAccessorResult.finalMemberVal.IsValid() )
				{
					return;
				}

				std::string lastMemberName = memberAccessorResult.memberNames.back();

				//if ( memberAccessorResult.finalMemberVal.GetType() == eValueType::ENTITY )
				//{
				//	SetGlobalVariableInEntity( memberAccessorResult.finalMemberVal.GetAsEntity(), lastMemberName, constantValue );
				//}
				//else if ( memberAccessorResult.finalMemberVal.GetType() == eValueType::USER_TYPE )
				//{
					std::string typeName = memberAccessorResult.finalMemberVal.GetTypeName();
					ZephyrTypeMetadata* metadata = g_zephyrSubsystem->GetRegisteredUserType( typeName );
					if ( metadata == nullptr )
					{
						ReportError( Stringf( "Non-registered user type '%s' can not be assigned to", typeName.c_str() ) );
						return;
					}

					ZephyrTypeMemberVariable* memberVariable = metadata->GetMemberVariable( lastMemberName );
					if ( memberVariable == nullptr )
					{
						ReportError( Stringf( "Zephyr type '%s' does not have a member '%s'", typeName.c_str(), lastMemberName.c_str() ) );
						return;
					}

					if ( memberVariable->isReadonly )
					{
						ReportError( Stringf( "Can't set readonly member '%s' in user type '%s'", lastMemberName.c_str(), typeName.c_str() ) );
						return;
					}

					if ( !memberAccessorResult.finalMemberVal.SetMember( lastMemberName, constantValue ) )
					{
						ReportError( Stringf( "Zephyr type '%s' does not implement SetMember", typeName.c_str() ) );
						return;
					}
				//}

				PushConstant( constantValue );
			}
			break;

			case eOpCode::MEMBER_ACCESSOR:
			{
				MemberAccessorResult memberAccessorResult = ProcessResultOfMemberAccessor();

				if ( !memberAccessorResult.finalMemberVal.IsValid() )
				{
					ReportError( Stringf( "Member accessor failed" ) );
					return;
				}

				// Push final member to top of constant stack
				const std::string& lastMemberName = memberAccessorResult.memberNames.back();
				//int memberCount = (int)memberAccessorResult.memberNames.size();

				//case eValueType::ENTITY:
				//{
				//	ZephyrValue val = GetGlobalVariableFromEntity( memberAccessorResult.finalMemberVal.GetAsEntity(), lastMemberName );
				//	if ( IsErrorValue( val ) )
				//	{
				//		std::string entityVarName = memberCount > 1 ? memberAccessorResult.memberNames[memberCount - 2] : memberAccessorResult.baseObjName;

				//		ReportError( Stringf( "Variable '%s' is not a member of Entity '%s'", lastMemberName.c_str(), entityVarName.c_str() ) );
				//		return;
				//	}

				//	PushConstant( val );
				//}
				//break;

				std::string typeName = memberAccessorResult.finalMemberVal.GetTypeName();
				ZephyrTypeMetadata* metadata = g_zephyrSubsystem->GetRegisteredUserType( typeName );
				if ( metadata == nullptr )
				{
					ReportError( Stringf( "Can't access members of non-registered user type '%s'", typeName.c_str() ) );
					return;
				}

				if ( !metadata->HasMemberVariable( lastMemberName ) )
				{
					ReportError( Stringf( "Zephyr type '%s' does not have a member '%s'", typeName.c_str(), lastMemberName.c_str() ) );
					return;
				}

				ZephyrValue memberVal = memberAccessorResult.finalMemberVal.GetMember( lastMemberName );
				if ( !memberVal.IsValid() )
				{
					ReportError( Stringf( "Zephyr type '%s' does not implement GetMember for '%s'", typeName.c_str(), lastMemberName.c_str() ) );
					return;
				}

				PushConstant( memberVal );
			}
			break;

			case eOpCode::MEMBER_FUNCTION_CALL:
			{
				// Save identifier names to be updated with new values after call
				std::map<std::string, std::string> identifierToParamNames = GetCallerVariableToParamNamesFromParameters( "Member function call" );

				ZephyrArgs* args = new ZephyrArgs();
				args->SetValue( PARENT_ENTITY_STR, m_zephyrComponent.GetParentEntityId() );

				InsertParametersIntoEventArgs( *args );

				MemberAccessorResult memberAccessorResult = ProcessResultOfMemberAccessor();

				if ( !memberAccessorResult.finalMemberVal.IsValid() )
				{
					return;
				}

				//int memberCount = (int)memberAccessorResult.memberNames.size();

				if ( memberAccessorResult.finalMemberVal.IsEntity() )
				{
					CallMemberFunctionOnEntity( memberAccessorResult.finalMemberVal.GetAsEntity(), memberAccessorResult.memberNames.back(), args );
				}
				else
				{
					if ( !memberAccessorResult.finalMemberVal.IsValid() )
					{
						ReportError( Stringf( "Zephyr object '%s' is null, cannot access its methods", memberAccessorResult.baseObjName.c_str() ));
						return;
					}

					if ( !memberAccessorResult.finalMemberVal.CallMethod( memberAccessorResult.memberNames.back(), args ) )
					{
						ReportError( Stringf( "Zephyr object '%s' does not have a registered method '%s'", memberAccessorResult.baseObjName.c_str(), memberAccessorResult.memberNames.back().c_str() ) );
						return;
					}
				}
				
				// Set new values of identifier parameters
				UpdateIdentifierParameters( identifierToParamNames, *args );

				PTR_SAFE_DELETE( args );
			}
			break;

			case eOpCode::RETURN:
			{
				// Stop processing this bytecode chunk
				return;
			}
			break;

			case eOpCode::IF:
			{
				ZephyrValue expression = PopConstant();
				ZephyrValue ifBlockByteCount = PopConstant();

				// The if statement is false, jump over the bytes corresponding to that code block
				if ( !expression.EvaluateAsBool() )
				{
					int numIfBlockBytes = 0;
					ifBlockByteCount.TryToGetValueFrom<ZephyrNumber>( numIfBlockBytes );
					curByteIdx += numIfBlockBytes;
				}
			}
			break;

			case eOpCode::JUMP:
			{
				ZephyrValue numBytesToJumpVal = PopConstant();	

				int numBytesToJump = 0;
				numBytesToJumpVal.TryToGetValueFrom<ZephyrNumber>( numBytesToJump );
				curByteIdx += numBytesToJump;
			}
			break;

			case eOpCode::SET_SCOPE:
			{
				ZephyrValue scopeIdxVal = PopConstant();

				int scopeIdx = 0;
				scopeIdxVal.TryToGetValueFrom<ZephyrNumber>( scopeIdx );
				m_bytecodeChunk.SetScopeFromIdx( scopeIdx );
			}
			break;

			case eOpCode::AND:
			{
				ZephyrValue rightVal = PopConstant();
				ZephyrValue leftVal = PopConstant();

				if ( !leftVal.EvaluateAsBool() 
					 || !rightVal.EvaluateAsBool() )
				{
					PushConstant( ZephyrValue( false ) );
				}
				else
				{
					PushConstant( ZephyrValue( true ) );
				}
			}
			break;

			case eOpCode::OR:
			{
				ZephyrValue rightVal = PopConstant();
				ZephyrValue leftVal = PopConstant();

				if ( !leftVal.EvaluateAsBool()
					 && !rightVal.EvaluateAsBool() )
				{
					PushConstant( ZephyrValue( false ) );
				}
				else
				{
					PushConstant( ZephyrValue( true ) );
				}
			}
			break;

			case eOpCode::NEGATE:
			{
				ZephyrValue a = PopConstant();
				
				ZephyrValue result = a.Negate();
				if ( !result.IsValid() )
				{
					ReportError( Stringf( "ZephyrType '%s' doesn't define a Negate function", a.GetTypeName().c_str() ) );
					return;
				}

				PushConstant( result );
			}
			break;

			case eOpCode::NOT:
			{
				ZephyrValue a = PopConstant();
				bool result = a.EvaluateAsBool();
					
				PushConstant( ZephyrValue( !result ) );
			}
			break;

			case eOpCode::ADD:
			case eOpCode::SUBTRACT:
			case eOpCode::MULTIPLY:
			case eOpCode::DIVIDE:
			case eOpCode::NOT_EQUAL:
			case eOpCode::EQUAL:
			case eOpCode::GREATER:
			case eOpCode::GREATER_EQUAL:
			case eOpCode::LESS:
			case eOpCode::LESS_EQUAL:
			{
				ZephyrValue b = PopConstant();
				ZephyrValue a = PopConstant();
				PushBinaryOp( a, b, opCode );
			}
			break;

			case eOpCode::FUNCTION_CALL:
			{
				ZephyrValue eventName = PopConstant();
				GUARANTEE_OR_DIE( eventName.GetTypeName() == ZephyrString::TYPE_NAME, "Event name isn't a string" );

				// Save identifier names to be updated with new values after call
				std::map<std::string, std::string> identifierToParamNames = GetCallerVariableToParamNamesFromParameters( eventName.ToString() );

				ZephyrArgs* args = new ZephyrArgs();
				args->SetValue( PARENT_ENTITY_STR, m_zephyrComponent.GetParentEntityId() );

				InsertParametersIntoEventArgs( *args );

				// Try to call GameAPI function, then local function
				if ( g_zephyrAPI->IsMethodRegistered( eventName.ToString() ) )
				{
					g_eventSystem->FireEvent( eventName.ToString(), args, EVERYWHERE );
				}
				else
				{
					if ( !CallMemberFunctionOnEntity( m_zephyrComponent.GetParentEntityId(), eventName.ToString(), args ) )
					{
						ReportError( Stringf( "Entity '%s' doesn't have a function '%s'", m_zephyrComponent.GetParentEntityName().c_str(), eventName.ToString().c_str() ) );
					}
				}

				// Set new values of identifier parameters
				UpdateIdentifierParameters( identifierToParamNames, *args );

				PTR_SAFE_DELETE( args );
			}
			break;

			case eOpCode::CHANGE_STATE:
			{
				ZephyrValue stateName = PopConstant();
				
				ZephyrSystem::ChangeZephyrScriptState( m_zephyrComponent.GetParentEntityId(), stateName.ToString() );

				// Bail out of this chunk to avoid trying to execute bytecode in the wrong update chunk
				return;
			}
			break;

			default:
			{
			}
			break;
		}
	}

	//m_bytecodeChunk.SetScopeFromIdx( 0 );
	CopyZephyrVariablesToEventArgs( m_eventArgs );
}


//-----------------------------------------------------------------------------------------------
// TODO: Convert this to only using ZephyrValue?
void ZephyrVirtualMachine::CopyEventArgsToZephyrVariables( ZephyrArgs* eventArgs )
{
	if ( eventArgs == nullptr )
	{
		return;
	}

	// For each event arg that matches a known ZephyrType, add it to the local variables map so the event can use it
	std::map<std::string, TypedPropertyBase*> argKeyValuePairs = eventArgs->GetAllKeyValuePairs();

	for ( auto const& keyValuePair : argKeyValuePairs )
	{
		if ( keyValuePair.second->Is<float>() )
		{
			m_bytecodeChunk.SetVariable( keyValuePair.first, ZephyrValue( eventArgs->GetValue( keyValuePair.first, 0.f ) ) );
		}
		else if ( keyValuePair.second->Is<EntityId>() )
		{
			m_bytecodeChunk.SetVariable( keyValuePair.first, ZephyrValue( eventArgs->GetValue( keyValuePair.first, (EntityId)ERROR_ZEPHYR_ENTITY_ID ) ) );
		}
		else if ( keyValuePair.second->Is<double>() )
		{
			m_bytecodeChunk.SetVariable( keyValuePair.first, ZephyrValue( (float)eventArgs->GetValue( keyValuePair.first, 0.0 ) ) );
		}
		else if ( keyValuePair.second->Is<bool>() )
		{
			m_bytecodeChunk.SetVariable( keyValuePair.first, ZephyrValue( eventArgs->GetValue( keyValuePair.first, false ) ) );
		}
		else if ( keyValuePair.second->Is<std::string>() 
				  || keyValuePair.second->Is<char*>() )
		{
			m_bytecodeChunk.SetVariable( keyValuePair.first, ZephyrValue( keyValuePair.second->GetAsString() ) );
		}
		else if ( keyValuePair.second->Is<ZephyrHandle>() )
		{
			TypedProperty<ZephyrHandle>* paramAsProperty = (TypedProperty<ZephyrHandle>*)keyValuePair.second;
			m_bytecodeChunk.SetVariable( keyValuePair.first, ZephyrValue( paramAsProperty->m_value ) );
		}

		// Any other variables will be ignored since they have no ZephyrType equivalent
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::CopyZephyrVariablesToEventArgs( ZephyrArgs* args )
{
	std::vector<std::string> zephyrValueOutParamNames;

	// Save updated event variables back into args
	if ( args != nullptr )
	{
		std::map<std::string, TypedPropertyBase*> argKeyValuePairs = args->GetAllKeyValuePairs();

		for ( auto const& keyValuePair : argKeyValuePairs )
		{
			ZephyrValue val;
			if ( !m_bytecodeChunk.TryToGetVariable( keyValuePair.first, val ) )
			{
				ReportError( Stringf( "Couldn't find local variable %s to save back into event arg", keyValuePair.first.c_str() ) );
				continue;
			}

			args->SetValue( keyValuePair.first, val );
			zephyrValueOutParamNames.push_back( keyValuePair.first );			
		}
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushConstant( const ZephyrValue& value )
{
	m_constantStack.push( value );
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVirtualMachine::PopConstant()
{
	GUARANTEE_OR_DIE( !m_constantStack.empty(), Stringf( "Constant stack is empty in script '%s'", m_zephyrComponent.GetScriptName().c_str() ) );
	
	ZephyrValue topConstant = m_constantStack.top();
	m_constantStack.pop();

	return topConstant;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVirtualMachine::PeekConstant()
{
	GUARANTEE_OR_DIE( !m_constantStack.empty(), Stringf( "Constant stack is empty in script '%s'", m_zephyrComponent.GetScriptName().c_str() ) );

	return m_constantStack.top();
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushBinaryOp( ZephyrValue& a, ZephyrValue& b, eOpCode opCode )
{
	switch ( opCode )
	{
		case eOpCode::ADD:				PushAddOp( a, b ); break;
		case eOpCode::SUBTRACT:			PushSubtractOp( a, b ); break;
		case eOpCode::MULTIPLY:			PushMultiplyOp( a, b ); break;
		case eOpCode::DIVIDE:			PushDivideOp( a, b ); break;
		case eOpCode::NOT_EQUAL:		PushNotEqualOp( a, b ); break;
		case eOpCode::EQUAL:			PushEqualOp( a, b ); break;
		case eOpCode::GREATER:			PushGreaterOp( a, b ); break;
		case eOpCode::GREATER_EQUAL:	PushGreaterEqualOp( a, b ); break;
		case eOpCode::LESS:				PushLessOp( a, b ); break;
		case eOpCode::LESS_EQUAL:		PushLessEqualOp( a, b ); break;
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushAddOp( ZephyrValue& a, ZephyrValue& b )
{		
	// Handle ZephyrString concatenation with other types
	if ( a.GetTypeName() == ZephyrString::TYPE_NAME
		|| b.GetTypeName() == ZephyrString::TYPE_NAME )
	{
		PushConstant( ZephyrValue( a.ToString() + b.ToString() ) );
		return;
	}

	ZephyrValue result = a.Add( b );
	if ( result.IsValid() )
	{
		PushConstant( result );
	}
	else
	{
		ReportError( Stringf( "%s + %s is undefined", a.GetTypeName().c_str(), b.GetTypeName().c_str() ) );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushSubtractOp( ZephyrValue& a, ZephyrValue& b )
{
	ZephyrValue result = a.Subtract( b );
	if ( result.IsValid() )
	{
		PushConstant( result );
	}
	else
	{
		ReportError( Stringf( "%s - %s is undefined", a.GetTypeName().c_str(), b.GetTypeName().c_str() ) );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushMultiplyOp( ZephyrValue& a, ZephyrValue& b )
{
	ZephyrValue result = a.Multiply( b );
	if ( result.IsValid() )
	{
		PushConstant( result );
	}
	else
	{
		ReportError( Stringf( "%s * %s is undefined", a.GetTypeName().c_str(), b.GetTypeName().c_str() ) );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushDivideOp( ZephyrValue& a, ZephyrValue& b )
{
	ZephyrValue result = a.Divide( b );
	if ( result.IsValid() )
	{
		PushConstant( result );
	}
	else
	{
		ReportError( Stringf( "%s / %s is undefined", a.GetTypeName().c_str(), b.GetTypeName().c_str() ) );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushNotEqualOp( ZephyrValue& a, ZephyrValue& b )
{
	ZephyrValue result = a.NotEqual( b );
	if ( result.IsValid() )
	{
		PushConstant( result );
	}
	else
	{
		ReportError( Stringf( "%s != %s is undefined", a.GetTypeName().c_str(), b.GetTypeName().c_str() ) );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushEqualOp( ZephyrValue& a, ZephyrValue& b )
{
	ZephyrValue result = a.Equal( b );
	if ( result.IsValid() )
	{
		PushConstant( result );
	}
	else
	{
		ReportError( Stringf( "%s == %s is undefined", a.GetTypeName().c_str(), b.GetTypeName().c_str() ) );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushGreaterOp( ZephyrValue& a, ZephyrValue& b )
{
	ZephyrValue result = a.Greater( b );
	if ( result.IsValid() )
	{
		PushConstant( result );
	}
	else
	{
		ReportError( Stringf( "%s > %s is undefined", a.GetTypeName().c_str(), b.GetTypeName().c_str() ) );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushGreaterEqualOp( ZephyrValue& a, ZephyrValue& b )
{
	ZephyrValue result = a.GreaterEqual( b );
	if ( result.IsValid() )
	{
		PushConstant( result );
	}
	else
	{
		ReportError( Stringf( "%s >= %s is undefined", a.GetTypeName().c_str(), b.GetTypeName().c_str() ) );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushLessOp( ZephyrValue& a, ZephyrValue& b )
{
	ZephyrValue result = a.Less( b );
	if ( result.IsValid() )
	{
		PushConstant( result );
	}
	else
	{
		ReportError( Stringf( "%s < %s is undefined", a.GetTypeName().c_str(), b.GetTypeName().c_str() ) );
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushLessEqualOp( ZephyrValue& a, ZephyrValue& b )
{
	ZephyrValue result = a.LessEqual( b );
	if ( result.IsValid() )
	{
		PushConstant( result );
	}
	else
	{
		ReportError( Stringf( "%s <= %s is undefined", a.GetTypeName().c_str(), b.GetTypeName().c_str() ) );
	}
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVirtualMachine::GetVariableValue( const std::string& variableName )
{
	ZephyrValue returnVal;
	if ( m_bytecodeChunk.TryToGetVariable( variableName, returnVal ) )
	{
		return returnVal;
	}

	ReportError( Stringf( "Variable '%s' is undefined", variableName.c_str() ) );
	return ZephyrValue();
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::AssignToVariable( const std::string& variableName, const ZephyrValue& value )
{
	m_bytecodeChunk.SetVariable( variableName, value );

	//ReportError( Stringf( "Cannot assign a value of type '%s' to variable '%s' of type '%s'", ToString( value.GetType() ).c_str(),
	//							variableName.c_str(),
	//							ToString( ( *variableMap )[variableName].GetType() ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
MemberAccessorResult ZephyrVirtualMachine::ProcessResultOfMemberAccessor()
{
	MemberAccessorResult memberAccessResult;

	ZephyrValue memberCountZephyr = PopConstant();
	//ZephyrNumberPtr memberCountVal = memberCountZephyr.GetAsPtrTo<ZephyrNumber>();
	int memberCount = 0;
	memberCountZephyr.TryToGetValueFrom<ZephyrNumber>( memberCount );


	ZephyrValue baseObjName = PopConstant();

	// Pull all members from the constant stack into a temp buffer for processing 
	// so in case there is a member error the stack is left in a good(ish) state 
	// Note: the members will be in reverse order, so account for that
	std::vector<std::string> memberNames;
	memberNames.resize( memberCount );
	for ( int memberIdx = memberCount - 1; memberIdx >= 0; --memberIdx )
	{
		memberNames[memberIdx] = PopConstant().ToString();
	}

	// Find base object in this bytecode chunk
	ZephyrValue memberVal = GetVariableValue( baseObjName.ToString() );
	if ( !memberVal.IsValid() )
	{
		return memberAccessResult;
	}

	std::vector<EntityId> entityIdChain;

	// Process accessors excluding the final component, since that needs to be handled separately
	for ( int memberNameIdx = 0; memberNameIdx < (int)memberNames.size() - 1; ++memberNameIdx )
	{
		std::string& memberName = memberNames[memberNameIdx];

		//case eValueType::ENTITY:
		//{
		//	ZephyrValue val = GetGlobalVariableFromEntity( memberVal.GetAsEntity(), memberName );
		//	if ( !val.IsValid() )
		//	{
		//		std::string entityVarName = memberNameIdx > 0 ? memberNames[memberNameIdx - 1] : baseObjName.ToString();

		//		ReportError( Stringf( "Variable '%s' is not a member of Entity '%s'", memberName.c_str(), entityVarName.c_str() ) );
		//		return memberAccessResult;
		//	}

		//	entityIdChain.push_back( memberVal.GetAsEntity() );
		//	memberVal = val;
		//}
		//break;

		if ( !memberVal.IsValid() )
		{
			ReportError( Stringf( "Variable '%s' is null, cannot dereference it", memberName.c_str() ) );
			return memberAccessResult;
		}

		memberVal = memberVal.GetMember( memberName );
		//memberVal = ZephyrValue( userTypeObj );	
	}
	

	memberAccessResult.finalMemberVal = memberVal;
	memberAccessResult.baseObjName = baseObjName.ToString();
	memberAccessResult.memberNames = memberNames;
	memberAccessResult.entityIdChain = entityIdChain;

	return memberAccessResult;
}


//-----------------------------------------------------------------------------------------------
std::map<std::string, std::string> ZephyrVirtualMachine::GetCallerVariableToParamNamesFromParameters( const std::string& eventName )
{
	std::map<std::string, std::string> identifierToParamNames;

	ZephyrValue identifierCountVal = PopConstant();

	int numIdentifiers = 0;
	identifierCountVal.TryToGetValueFrom<ZephyrNumber>( numIdentifiers );

	for ( int identifierIdx = 0; identifierIdx < numIdentifiers; ++identifierIdx )
	{
		ZephyrValue identifierParamName = PopConstant();
		ZephyrValue identifier = PopConstant();
		if ( identifier.GetTypeName() != ZephyrString::TYPE_NAME
			 || identifierParamName.GetTypeName() != ZephyrString::TYPE_NAME )
		{
			ReportError( Stringf( "Identifier name is not a string in parameter list for function call '%s'", eventName.c_str() ) );
			continue;
		}
		identifierToParamNames[identifier.ToString()] = identifierParamName.ToString();
	}

	return identifierToParamNames;
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::InsertParametersIntoEventArgs( ZephyrArgs& args )
{
	ZephyrValue paramCount = PopConstant();

	int numOfParams = 0;
	paramCount.TryToGetValueFrom<ZephyrNumber>( numOfParams );

	for ( int paramIdx = 0; paramIdx < numOfParams; ++paramIdx )
	{
		ZephyrValue param = PopConstant();
		ZephyrValue value = PopConstant();

		args.SetValue( param.ToString(), value );

		//switch ( value.GetType() )
		//{
		//	case eValueType::BOOL:		args.SetValue( param.GetAsString(), value.GetAsBool() ); break;
		//	case eValueType::NUMBER:	args.SetValue( param.GetAsString(), value.GetAsNumber() ); break;
		//	case eValueType::STRING:	args.SetValue( param.GetAsString(), value.GetAsString() ); break;
		//	case eValueType::ENTITY:	args.SetValue( param.GetAsString(), value.GetAsEntity() ); break;
		//	case eValueType::USER_TYPE:	args.SetValue( param.GetAsString(), value.EvaluateAsUserType() ); break;
		//	default: ERROR_AND_DIE( Stringf( "Unimplemented event arg type '%s'", ToString( value.GetType() ).c_str() ) );
		//}
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::UpdateIdentifierParameters( const std::map<std::string, std::string>& identifierToParamNames, const ZephyrArgs& args )
{
	for ( auto const& identifierPair : identifierToParamNames )
	{
		std::string const& identifier = identifierPair.first;
		std::string const& paramName = identifierPair.second;

		ZephyrValue newVal = GetZephyrValFromEventArgs( paramName, args );
		
		if ( !newVal.IsValid() )
		{
			//ReportError( Stringf( "No matching arg found for parameter '%s'", param.c_str() ) );
			continue;
		}
		
		AssignToVariable( identifier, newVal );
	}
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVirtualMachine::GetZephyrValFromEventArgs( const std::string& varName, const ZephyrArgs& args )
{
	auto const& keyValuePairs = args.GetAllKeyValuePairs();
	auto iter = keyValuePairs.find( varName );
	if ( iter == keyValuePairs.end() )
	{
		return ZephyrValue::NULL_VAL;
	}
	
	ZephyrArgs params;
	if ( iter->second->Is<float>() )
	{
		return ZephyrValue( args.GetValue( varName, 0.f ) );
	}
	else if ( iter->second->Is<double>() )
	{
		return ZephyrValue( (float)args.GetValue( varName, 0.0 ) );
	}
	else if ( iter->second->Is<EntityId>() )
	{
		return ZephyrValue( args.GetValue( varName, (EntityId)ERROR_ZEPHYR_ENTITY_ID ) );
	}
	else if ( iter->second->Is<bool>() )
	{
		return ZephyrValue( args.GetValue( varName, false ) );
	}
	else if ( iter->second->Is<std::string>()
			  || iter->second->Is<char*>() )
	{
		return ZephyrValue( iter->second->GetAsString() );
	}
	else if ( iter->second->Is<ZephyrHandle>() )
	{
		return ZephyrValue( args.GetValue( varName, NULL_ZEPHYR_HANDLE ) );
	}

	return ZephyrValue::NULL_VAL;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVirtualMachine::GetGlobalVariableFromEntity( EntityId entityId, const std::string& variableName )
{
	return ZephyrSystem::GetGlobalVariable( entityId, variableName );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::SetGlobalVariableInEntity( EntityId entityId, const std::string& variableName, const ZephyrValue& value )
{
	return ZephyrSystem::SetGlobalVariable( entityId, variableName, value );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrVirtualMachine::CallMemberFunctionOnEntity( EntityId entityId, const std::string& functionName, ZephyrArgs* args )
{
	ZephyrComponent* zephyrComp = (ZephyrComponent*)GetComponentFromEntityId( entityId, ENTITY_COMPONENT_TYPE_ZEPHYR );
	if ( zephyrComp == nullptr )
	{
		ReportError( Stringf( "Entity '%s' does not have a zephyr component", Entity::GetName(entityId).c_str() ) );
		return false;
	}

	// Must be zephyr component of entityId
	return ZephyrSystem::FireScriptEvent( zephyrComp, functionName, args );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::ReportError( const std::string& errorMsg )
{
	//m_bytecodeChunk.Disassemble();

	g_devConsole->PrintError( Stringf( "Error in zephyr script '%s'", m_zephyrComponent.GetScriptNameWithExtension().c_str() ) );

	std::string errorPrefix;
	/*if ( ZephyrBytecodeChunk* parentChunk = m_bytecodeChunk.GetParentChunk() )
	{
		if ( parentChunk->GetType() == eBytecodeChunkType::STATE )
		{
			errorPrefix.append( parentChunk->GetName() );
			errorPrefix.append( "::" );
		}
	}*/
	
	errorPrefix.append( m_bytecodeChunk.GetFullyQualifiedName() );

	ZephyrScriptDefinition* zephyrScriptDef = ZephyrScriptDefinition::GetZephyrScriptDefinitionByName( m_zephyrComponent.GetScriptNameWithExtension() );
	int lineNum = zephyrScriptDef->GetLineNumFromOpCodeIdx( m_bytecodeChunk.GetFullyQualifiedName(), m_curOpCodeIdx );
	
	g_devConsole->PrintError( Stringf( "  %s line %i: %s", errorPrefix.c_str(), lineNum, errorMsg.c_str() ) );

	m_zephyrComponent.m_compState = eComponentState::INVALID_SCRIPT;
}
