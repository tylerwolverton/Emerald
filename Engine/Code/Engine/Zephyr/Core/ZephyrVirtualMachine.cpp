#include "Engine/Zephyr/Core/ZephyrVirtualMachine.hpp"
#include "Engine/Zephyr/Core/ZephyrBytecodeChunk.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrComponent.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrEngineEvents.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSystem.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Framework/Entity.hpp"


//-----------------------------------------------------------------------------------------------
ZephyrVirtualMachine::ZephyrVirtualMachine( ZephyrValueMap* globalVariables, 
											ZephyrComponent& zephyrComponent, 
											ZephyrArgs* eventArgs,
											ZephyrValueMap* stateVariables )
	: m_globalVariables( globalVariables )
	, m_zephyrComponent( zephyrComponent )
	, m_stateVariables( stateVariables )
	, m_eventArgs( eventArgs )
{
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::InterpretBytecodeChunk( const ZephyrBytecodeChunk& bytecodeChunk )
{
	if ( !m_zephyrComponent.IsScriptValid() )
	{
		return;
	}

	// Event variables don't need to be persisted after this call, so save a copy as local variables
	// TODO: Account for scopes inside if statements, etc.?
	std::map<std::string, ZephyrValue> localVariables;
	if (  bytecodeChunk.GetType() == eBytecodeChunkType::EVENT )
	{
		localVariables = bytecodeChunk.GetVariables();
		CopyEventArgVariables( m_eventArgs, localVariables );
	}

	int byteIdx = 0;
	while ( byteIdx < bytecodeChunk.GetNumBytes() )
	{
		// If this script has an error during interpretation, bail out to avoid running in a broken, unknown state
		if ( !m_zephyrComponent.IsScriptValid() )
		{
			return;
		}

		byte instruction = bytecodeChunk.GetByte( byteIdx++ );
		eOpCode opCode = ByteToOpCode( instruction );
		switch ( opCode )
		{
			case eOpCode::CONSTANT:
			{
				int constIdx = bytecodeChunk.GetByte( byteIdx++ );
				ZephyrValue constant = bytecodeChunk.GetConstant( constIdx );
				PushConstant( constant );
			}
			break;			
			
			case eOpCode::CONSTANT_USER_TYPE:
			{
				ZephyrValue variableName = PopConstant();
				ZephyrValue variableType = PopConstant();

				ZephyrArgs args;
				InsertParametersIntoEventArgs( args );

				// TODO: Delete this somewhere
				PushConstant( ZephyrValue( g_zephyrTypeObjFactory->CreateObject( variableType.GetAsString(), &args ) ) );
			}
			break;

			case eOpCode::DEFINE_VARIABLE:
			{
				ZephyrValue variableName = PopConstant();
				localVariables[variableName.GetAsString()] = PopConstant();
			}
			break;

			case eOpCode::GET_VARIABLE_VALUE:
			{
				ZephyrValue variableName = PopConstant();
				PushConstant( GetVariableValue( variableName.GetAsString(), localVariables ) );
			}
			break;
			
			case eOpCode::ASSIGNMENT:
			{
				ZephyrValue variableName = PopConstant(); 
				ZephyrValue constantValue = PeekConstant();
				AssignToVariable( variableName.GetAsString(), constantValue, localVariables );
			}
			break;	

			case eOpCode::MEMBER_ASSIGNMENT:
			{
				ZephyrValue constantValue = PopConstant();

				MemberAccessorResult memberAccessorResult = ProcessResultOfMemberAccessor( localVariables );

				if ( IsErrorValue( memberAccessorResult.finalMemberVal ) )
				{
					return;
				}

				std::string lastMemberName = memberAccessorResult.memberNames.back();

				if ( memberAccessorResult.finalMemberVal.GetType() == eValueType::ENTITY )
				{
					SetGlobalVariableInEntity( memberAccessorResult.finalMemberVal.GetAsEntity(), lastMemberName, constantValue );
				}
				else if ( memberAccessorResult.finalMemberVal.GetType() == eValueType::VEC2 )
				{
					if ( lastMemberName != "x"
						 && lastMemberName != "y" )
					{
						ReportError( Stringf( "'%s' is not a member of Vec2", lastMemberName.c_str() ) );
						return;
					}

					// This variable belongs to current entity, save like this to set local or state variables
					int memberCount = (int)memberAccessorResult.memberNames.size();
					if ( memberCount <= 1 )
					{
						AssignToVec2MemberVariable( memberAccessorResult.baseObjName, lastMemberName, constantValue, localVariables );
					}
					else
					{
						// Account for this being a member of a different entity
						EntityId entityIdWithMember = memberAccessorResult.entityIdChain.back();
						std::string vec2VarName = memberAccessorResult.memberNames[memberCount - 2];

						SetGlobalVec2MemberVariableInEntity( entityIdWithMember, vec2VarName, lastMemberName, constantValue );
					}
				}
				else if ( memberAccessorResult.finalMemberVal.GetType() == eValueType::VEC3 )
				{
					if ( lastMemberName != "x"
						 && lastMemberName != "y" 
						 && lastMemberName != "z" )
					{
						ReportError( Stringf( "'%s' is not a member of Vec3", lastMemberName.c_str() ) );
						return;
					}

					// This variable belongs to current entity, save like this to set local or state variables
					int memberCount = (int)memberAccessorResult.memberNames.size();
					if ( memberCount <= 1 )
					{
						AssignToVec3MemberVariable( memberAccessorResult.baseObjName, lastMemberName, constantValue, localVariables );
					}
					else
					{
						// Account for this being a member of a different entity
						EntityId entityIdWithMember = memberAccessorResult.entityIdChain.back();
						std::string vec3VarName = memberAccessorResult.memberNames[memberCount - 2];

						SetGlobalVec3MemberVariableInEntity( entityIdWithMember, vec3VarName, lastMemberName, constantValue );
					}
				}
				else if ( memberAccessorResult.finalMemberVal.GetType() == eValueType::USER_TYPE )
				{
					std::string typeName = memberAccessorResult.finalMemberVal.GetAsUserType()->GetTypeName();
					ZephyrTypeMetadata* metadata = g_zephyrSubsystem->GetRegisteredUserType( typeName );
					if ( metadata == nullptr )
					{
						ReportError( Stringf( "Non-registered user type '%s' can not be assigned to", typeName.c_str() ) );
						return;
					}

					if ( !metadata->DoesTypeHaveMemberVariable( lastMemberName ) )
					{
						ReportError( Stringf( "User type '%s' does not have a member '%s'", typeName.c_str(), lastMemberName.c_str() ) );
					}

					ZephyrArgs args;
					args.SetValue( "value", (ZephyrTypeBase*)constantValue.EvaluateAsUserType() );
					memberAccessorResult.finalMemberVal.GetAsUserType()->CallMethod( "Set_" + lastMemberName, &args );
				}

				PushConstant( constantValue );
			}
			break;

			case eOpCode::MEMBER_ACCESSOR:
			{
				MemberAccessorResult memberAccessorResult = ProcessResultOfMemberAccessor( localVariables );

				if ( IsErrorValue( memberAccessorResult.finalMemberVal ) )
				{
					return;
				}

				// Push final member to top of constant stack
				const std::string& lastMemberName = memberAccessorResult.memberNames.back();
				int memberCount = (int)memberAccessorResult.memberNames.size();

				switch ( memberAccessorResult.finalMemberVal.GetType() )
				{
					case eValueType::BOOL:
					case eValueType::NUMBER:
					case eValueType::STRING:
					{
						ReportError( Stringf( "Variable of type %s can't have members. Tried to access '%s'",
																ToString( memberAccessorResult.finalMemberVal.GetType() ).c_str(),
																lastMemberName.c_str() ) );
						return;
					}
					break;

					case eValueType::VEC2:
					{
						if		( lastMemberName == "x" ) { PushConstant( memberAccessorResult.finalMemberVal.GetAsVec2().x ); }
						else if ( lastMemberName == "y" ) { PushConstant( memberAccessorResult.finalMemberVal.GetAsVec2().y ); }
						else
						{
							ReportError( Stringf( "'%s' is not a member of Vec2", lastMemberName.c_str() ) );
							return;
						}
					}
					break;

					case eValueType::VEC3:
					{
						if		( lastMemberName == "x" ) { PushConstant( memberAccessorResult.finalMemberVal.GetAsVec3().x ); }
						else if ( lastMemberName == "y" ) { PushConstant( memberAccessorResult.finalMemberVal.GetAsVec3().y ); }
						else if ( lastMemberName == "z" ) { PushConstant( memberAccessorResult.finalMemberVal.GetAsVec3().z ); }
						else
						{
							ReportError( Stringf( "'%s' is not a member of Vec3", lastMemberName.c_str() ) );
							return;
						}
					}
					break;

					case eValueType::ENTITY:
					{
						ZephyrValue val = GetGlobalVariableFromEntity( memberAccessorResult.finalMemberVal.GetAsEntity(), lastMemberName );
						if ( IsErrorValue( val ) )
						{
							std::string entityVarName = memberCount > 1 ? memberAccessorResult.memberNames[memberCount - 2] : memberAccessorResult.baseObjName;

							ReportError( Stringf( "Variable '%s' is not a member of Entity '%s'", lastMemberName.c_str(), entityVarName.c_str() ) );
							return;
						}

						PushConstant( val );
					}
					break;

					case eValueType::USER_TYPE:
					{
						ReportError( Stringf( "Accessing members in user types is not implemented yet" ) );
						return;
					}
					break;
				}
			}
			break;

			case eOpCode::MEMBER_FUNCTION_CALL:
			{
				// Save identifier names to be updated with new values after call
				std::map<std::string, std::string> identifierToParamNames = GetCallerVariableToParamNamesFromParameters( "Member function call" );

				ZephyrArgs* args = new ZephyrArgs();
				args->SetValue( PARENT_ENTITY_ID_STR, m_zephyrComponent.GetParentEntityId() );

				InsertParametersIntoEventArgs( *args );

				MemberAccessorResult memberAccessorResult = ProcessResultOfMemberAccessor( localVariables );

				if ( IsErrorValue( memberAccessorResult.finalMemberVal ) )
				{
					return;
				}

				int memberCount = (int)memberAccessorResult.memberNames.size();

				if ( memberAccessorResult.finalMemberVal.GetType() == eValueType::ENTITY )
				{
					CallMemberFunctionOnEntity( memberAccessorResult.finalMemberVal.GetAsEntity(), memberAccessorResult.memberNames.back(), args );
				}
				else if ( memberAccessorResult.finalMemberVal.GetType() == eValueType::USER_TYPE )
				{
					ZephyrTypeBase* userTypeObj = memberAccessorResult.finalMemberVal.GetAsUserType();
					if ( userTypeObj == nullptr )
					{
						ReportError( Stringf( "User type object '%s' is null, cannot access its methods", memberAccessorResult.baseObjName.c_str() ));
						return;
					}

					CallMemberFunctionOnUserType( *userTypeObj, memberAccessorResult.memberNames.back(), args );
				}
				else
				{
					std::string entityVarName = memberCount > 1 ? memberAccessorResult.memberNames[memberCount - 2] : memberAccessorResult.baseObjName;
					ReportError( Stringf( "Cannot call method on non entity variable '%s' with type '%s'", entityVarName.c_str(), ToString( memberAccessorResult.finalMemberVal.GetType() ).c_str() ) );
					return;
				}
				
				// Set new values of identifier parameters
				UpdateIdentifierParameters( identifierToParamNames, *args, localVariables );

				PTR_SAFE_DELETE( args );
			}
			break;

			case eOpCode::CONSTANT_VEC2:
			{
				ZephyrValue yValue = PopConstant();
				ZephyrValue xValue = PopConstant();

				ZephyrValue value( Vec2( xValue.GetAsNumber(), yValue.GetAsNumber() ) );
				PushConstant( value );
			}
			break;

			case eOpCode::CONSTANT_VEC3:
			{
				ZephyrValue zValue = PopConstant();
				ZephyrValue yValue = PopConstant();
				ZephyrValue xValue = PopConstant();

				ZephyrValue value( Vec3( xValue.GetAsNumber(), yValue.GetAsNumber(), zValue.GetAsNumber() ) );
				PushConstant( value );
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
					byteIdx += (int)ifBlockByteCount.GetAsNumber();
				}
			}
			break;

			case eOpCode::JUMP:
			{
				ZephyrValue numBytesToJump = PopConstant();
				byteIdx += (int)numBytesToJump.GetAsNumber();
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
				if ( a.GetType() == eValueType::NUMBER )
				{
					PushConstant( -a.GetAsNumber() );
				}
				else if ( a.GetType() == eValueType::VEC2 )
				{
					PushConstant( -a.GetAsVec2() );
				}
				else if ( a.GetType() == eValueType::VEC3 )
				{
					PushConstant( -a.GetAsVec3() );
				}
			}
			break;

			case eOpCode::NOT:
			{
				ZephyrValue a = PopConstant();
				if ( a.GetType() == eValueType::NUMBER )
				{
					if ( IsNearlyEqual( a.GetAsNumber(), 0.f, .00001f ) )
					{
						PushConstant( ZephyrValue( true ) );
					}
					else
					{
						PushConstant( ZephyrValue( false ) );
					}
				}
				else if ( a.GetType() == eValueType::BOOL )
				{
					PushConstant( !a.GetAsBool() );
				}
				else if ( a.GetType() == eValueType::STRING )
				{
					if ( a.GetAsString().empty() )
					{
						PushConstant( ZephyrValue( true ) );
					}
					else
					{
						PushConstant( ZephyrValue( false ) );
					}
				}
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
				GUARANTEE_OR_DIE( eventName.GetType() == eValueType::STRING, "Event name isn't a string" );

				// Save identifier names to be updated with new values after call
				std::map<std::string, std::string> identifierToParamNames = GetCallerVariableToParamNamesFromParameters( eventName.GetAsString() );

				ZephyrArgs* args = new ZephyrArgs();
				args->SetValue( PARENT_ENTITY_ID_STR, m_zephyrComponent.GetParentEntityId() );

				InsertParametersIntoEventArgs( *args );

				// Try to call GameAPI function, then local function
				if ( g_zephyrAPI->IsMethodRegistered( eventName.GetAsString() ) )
				{
					g_eventSystem->FireEvent( eventName.GetAsString(), args, EVERYWHERE );
				}
				else
				{
					if ( !CallMemberFunctionOnEntity( m_zephyrComponent.GetParentEntityId(), eventName.GetAsString(), args ) )
					{
						ReportError( Stringf( "Entity '%s' doesn't have a function '%s'", m_zephyrComponent.GetParentEntityName().c_str(), eventName.GetAsString().c_str() ) );
					}
				}

				// Set new values of identifier parameters
				UpdateIdentifierParameters( identifierToParamNames, *args, localVariables );

				PTR_SAFE_DELETE( args );
			}
			break;

			case eOpCode::CHANGE_STATE:
			{
				ZephyrValue stateName = PopConstant();
				
				ZephyrSystem::ChangeZephyrScriptState( m_zephyrComponent.GetParentEntityId(), stateName.GetAsString() );

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

	// Save updated event variables back into args
	if ( m_eventArgs != nullptr )
	{
		std::map<std::string, TypedPropertyBase*> argKeyValuePairs = m_eventArgs->GetAllKeyValuePairs();

		for ( auto const& keyValuePair : argKeyValuePairs )
		{
			ZephyrValue const& val = localVariables[keyValuePair.first];

			switch ( val.GetType() )
			{
				//case eValueType::NUMBER:	m_eventArgs->SetValue( keyValuePair.first, val.GetAsNumber() ); break;
				case eValueType::VEC2:		m_eventArgs->SetValue( keyValuePair.first, val.GetAsVec2() ); break;
				case eValueType::VEC3:		m_eventArgs->SetValue( keyValuePair.first, val.GetAsVec3() ); break;
				//case eValueType::STRING:	m_eventArgs->SetValue( keyValuePair.first, val.GetAsString() ); break;
				case eValueType::ENTITY:	m_eventArgs->SetValue( keyValuePair.first, val.GetAsEntity() ); break;
				//case eValueType::BOOL:		m_eventArgs->SetValue( keyValuePair.first, val.GetAsBool() ); break;
			}
		}
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::CopyEventArgVariables( ZephyrArgs* eventArgs, ZephyrValueMap& localVariables )
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
			localVariables[keyValuePair.first] = ZephyrValue( eventArgs->GetValue( keyValuePair.first, 0.f ) );
		}
		else if ( keyValuePair.second->Is<EntityId>() )
		{
			localVariables[keyValuePair.first] = ZephyrValue( eventArgs->GetValue( keyValuePair.first, (EntityId)ERROR_ZEPHYR_ENTITY_ID ) );
		}
		else if ( keyValuePair.second->Is<double>() )
		{
			localVariables[keyValuePair.first] = ZephyrValue( (float)eventArgs->GetValue( keyValuePair.first, 0.0 ) );
		}
		else if ( keyValuePair.second->Is<bool>() )
		{
			localVariables[keyValuePair.first] = ZephyrValue( eventArgs->GetValue( keyValuePair.first, false ) );
		}
		else if ( keyValuePair.second->Is<Vec2>() )
		{
			localVariables[keyValuePair.first] = ZephyrValue( eventArgs->GetValue( keyValuePair.first, Vec2::ZERO ) );
		}
		else if ( keyValuePair.second->Is<Vec3>() )
		{
			localVariables[keyValuePair.first] = ZephyrValue( eventArgs->GetValue( keyValuePair.first, Vec3::ZERO ) );
		}
		else if ( keyValuePair.second->Is<std::string>() 
				  || keyValuePair.second->Is<char*>() )
		{
			localVariables[keyValuePair.first] = ZephyrValue( keyValuePair.second->GetAsString() );
		}
		else if ( keyValuePair.second->Is<ZephyrTypeBase*>() )
		{
			localVariables[keyValuePair.first] = ZephyrValue( (ZephyrTypeBase*)keyValuePair.second );
		}

		// Any other variables will be ignored since they have no ZephyrType equivalent
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushConstant( const ZephyrValue& number )
{
	m_constantStack.push( number );
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
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		NUMBER_TYPE result = a.GetAsNumber() + b.GetAsNumber();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::VEC2 && bType == eValueType::VEC2 )
	{
		Vec2 result = a.GetAsVec2() + b.GetAsVec2();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::VEC3 && bType == eValueType::VEC3 )
	{
		Vec3 result = a.GetAsVec3() + b.GetAsVec3();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::STRING
		 || bType == eValueType::STRING )
	{
		std::string result = a.EvaluateAsString();
		result.append( b.EvaluateAsString() );
		PushConstant( result );
		return;
	}

	ReportError( Stringf( "Cannot add a variable of type %s with a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushSubtractOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		NUMBER_TYPE result = a.GetAsNumber() - b.GetAsNumber();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::VEC2 && bType == eValueType::VEC2 )
	{
		Vec2 result = a.GetAsVec2() - b.GetAsVec2();
		PushConstant( result );
		return;
	}
	
	if ( aType == eValueType::VEC3 && bType == eValueType::VEC3 )
	{
		Vec3 result = a.GetAsVec3() - b.GetAsVec3();
		PushConstant( result );
		return;
	}

	ReportError( Stringf( "Cannot subtract a variable of type %s from a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrVirtualMachine::TryToPushVec2MultiplyOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::VEC2 && bType == eValueType::VEC2 )
	{
		Vec2 result = a.GetAsVec2() * b.GetAsVec2();
		PushConstant( result );
		return true;
	}

	if ( aType == eValueType::VEC2 && bType == eValueType::NUMBER )
	{
		Vec2 result = a.GetAsVec2() * b.GetAsNumber();
		PushConstant( result );
		return true;
	}

	if ( aType == eValueType::NUMBER && bType == eValueType::VEC2 )
	{
		Vec2 result = a.GetAsNumber() * b.GetAsVec2();
		PushConstant( result );
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrVirtualMachine::TryToPushVec3MultiplyOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::VEC3 && bType == eValueType::VEC3 )
	{
		Vec3 result = a.GetAsVec3() * b.GetAsVec3();
		PushConstant( result );
		return true;
	}

	if ( aType == eValueType::VEC3 && bType == eValueType::NUMBER )
	{
		Vec3 result = a.GetAsVec3() * b.GetAsNumber();
		PushConstant( result );
		return true;
	}

	if ( aType == eValueType::NUMBER && bType == eValueType::VEC3 )
	{
		Vec3 result = a.GetAsNumber() * b.GetAsVec3();
		PushConstant( result );
		return true;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushMultiplyOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		NUMBER_TYPE result = a.GetAsNumber() * b.GetAsNumber();
		PushConstant( result );
		return;
	}

	if ( TryToPushVec2MultiplyOp( a, b ) ) return;
	if ( TryToPushVec3MultiplyOp( a, b ) ) return;

	ReportError( Stringf( "Cannot multiply a variable of type %s by a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushDivideOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		if ( IsNearlyEqual( b.GetAsNumber(), 0.f, .0000001f ) )
		{
			ReportError( "Cannot divide a Number variable by 0" );
			return;
		}

		NUMBER_TYPE result = a.GetAsNumber() / b.GetAsNumber();
		PushConstant( result );
		return;
	}
	
	if ( aType == eValueType::VEC2 && bType == eValueType::NUMBER )
	{
		if ( IsNearlyEqual( b.GetAsNumber(), 0.f, .0000001f ) )
		{
			ReportError( "Cannot divide a Vec2 variable by 0" );
			return;
		}

		Vec2 result = a.GetAsVec2() / b.GetAsNumber();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::VEC3 && bType == eValueType::NUMBER )
	{
		if ( IsNearlyEqual( b.GetAsNumber(), 0.f, .0000001f ) )
		{
			ReportError( "Cannot divide a Vec3 variable by 0" );
			return;
		}

		Vec3 result = a.GetAsVec3() / b.GetAsNumber();
		PushConstant( result );
		return;
	}
	
	ReportError( Stringf( "Cannot divide a variable of type %s by a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushNotEqualOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		bool result = !IsNearlyEqual( a.GetAsNumber(), b.GetAsNumber(), .001f );
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::VEC2 && bType == eValueType::VEC2 )
	{
		bool result = !IsNearlyEqual( a.GetAsVec2(), b.GetAsVec2(), .001f );
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::VEC3 && bType == eValueType::VEC3 )
	{
		bool result = !IsNearlyEqual( a.GetAsVec3(), b.GetAsVec3(), .001f );
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::STRING && bType == eValueType::STRING )
	{
		bool result = a.GetAsString() != b.GetAsString();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::BOOL 
		 || bType == eValueType::BOOL )
	{
		bool result = a.EvaluateAsBool() != b.EvaluateAsBool();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::ENTITY && bType == eValueType::ENTITY )
	{
		bool result = a.GetAsEntity() != b.GetAsEntity();
		PushConstant( result );
		return;
	}

	ReportError( Stringf( "Cannot compare a variable of type %s with a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushEqualOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		bool result = IsNearlyEqual( a.GetAsNumber(), b.GetAsNumber(), .001f );
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::VEC2 && bType == eValueType::VEC2 )
	{
		bool result = IsNearlyEqual( a.GetAsVec2(), b.GetAsVec2(), .001f );
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::VEC3 && bType == eValueType::VEC3 )
	{
		bool result = IsNearlyEqual( a.GetAsVec3(), b.GetAsVec3(), .001f );
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::STRING && bType == eValueType::STRING )
	{
		bool result = a.GetAsString() == b.GetAsString();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::BOOL
		 || bType == eValueType::BOOL )
	{
		bool result = a.EvaluateAsBool() == b.EvaluateAsBool();
		PushConstant( result );
		return;
	}

	if ( aType == eValueType::ENTITY && bType == eValueType::ENTITY )
	{
		bool result = a.GetAsEntity() == b.GetAsEntity();
		PushConstant( result );
		return;
	}

	ReportError( Stringf( "Cannot compare a variable of type %s with a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushGreaterOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		bool result = a.GetAsNumber() > b.GetAsNumber();
		PushConstant( result );
		return;
	}
	
	ReportError( Stringf( "Cannot check if a variable of type %s is greater than a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushGreaterEqualOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		bool result = a.GetAsNumber() < b.GetAsNumber();
		PushConstant( !result );
		return;
	}

	ReportError( Stringf( "Cannot check if a variable of type %s is greater than or equal to a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushLessOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		bool result = a.GetAsNumber() < b.GetAsNumber();
		PushConstant( result );
		return;
	}

	ReportError( Stringf( "Cannot check if a variable of type %s is less than a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::PushLessEqualOp( ZephyrValue& a, ZephyrValue& b )
{
	eValueType aType = a.GetType();
	eValueType bType = b.GetType();

	if ( aType == eValueType::NUMBER && bType == eValueType::NUMBER )
	{
		bool result = a.GetAsNumber() > b.GetAsNumber();
		PushConstant( !result );
		return;
	}

	ReportError( Stringf( "Cannot check if a variable of type %s is less than a variable of type %s", ToString( aType ).c_str(), ToString( bType ).c_str() ) );
}


//-----------------------------------------------------------------------------------------------
// TODO: Clean this up with ZephyrScope or bytecode chunk parents
ZephyrValue ZephyrVirtualMachine::GetVariableValue( const std::string& variableName, const ZephyrValueMap& localVariables )
{
	// Try to find in local variables first
	auto localIter = localVariables.find( variableName );
	if ( localIter != localVariables.end() )
	{
		return localIter->second;
	}

	// Check event args
	if ( !m_eventVariablesCopy.empty() )
	{
		auto eventIter = m_eventVariablesCopy.find( variableName );
		if ( eventIter != m_eventVariablesCopy.end() )
		{
			return eventIter->second;
		}
	}

	// Check state variables
	if ( m_stateVariables != nullptr )
	{
		auto stateIter = m_stateVariables->find( variableName );
		if ( stateIter != m_stateVariables->end() )
		{
			return stateIter->second;
		}
	}

	// Check global variables
	if ( m_globalVariables != nullptr )
	{
		auto globalIter = m_globalVariables->find( variableName );
		if ( globalIter != m_globalVariables->end() )
		{
			return globalIter->second;
		}
	}

	ReportError( Stringf( "Variable '%s' is undefined", variableName.c_str() ) );
	return ZephyrValue::ERROR_VALUE;
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::AssignToVariable( const std::string& variableName, const ZephyrValue& value, ZephyrValueMap& localVariables )
{
	EAssignToVariableInMapResult assignResult = AssignToVariableInMap( variableName, value, &localVariables );
	if ( assignResult == EAssignToVariableInMapResult::ERROR
		 || assignResult == EAssignToVariableInMapResult::SUCCESS )
	{
		return;
	}
	
	assignResult = AssignToVariableInMap( variableName, value, m_stateVariables );
	if ( assignResult == EAssignToVariableInMapResult::ERROR
		 || assignResult == EAssignToVariableInMapResult::SUCCESS )
	{
		return;
	}

	AssignToVariableInMap( variableName, value, m_globalVariables );
}


//-----------------------------------------------------------------------------------------------
EAssignToVariableInMapResult ZephyrVirtualMachine::AssignToVariableInMap( const std::string& variableName, const ZephyrValue& value, ZephyrValueMap* variableMap )
{
	if ( variableMap == nullptr )
	{
		return EAssignToVariableInMapResult::NOT_FOUND;
	}

	auto mapIter = variableMap->find( variableName );
	if ( mapIter == variableMap->end() )
	{
		return EAssignToVariableInMapResult::NOT_FOUND;
	}

	if ( mapIter->second.GetType() != value.GetType() )
	{
		ReportError( Stringf( "Cannot assign a value of type '%s' to variable '%s' of type '%s'", ToString( value.GetType() ).c_str(),
								variableName.c_str(),
								ToString( ( *variableMap )[variableName].GetType() ).c_str() ) );
		return EAssignToVariableInMapResult::ERROR;
	}

	( *variableMap )[variableName] = value;
	return EAssignToVariableInMapResult::SUCCESS;
}


//-----------------------------------------------------------------------------------------------
// TODO: Find a more general way to set member variables
// ZEPHYR TYPE TODO: Remove/codegen
void ZephyrVirtualMachine::AssignToVec2MemberVariable( const std::string& variableName, const std::string& memberName, const ZephyrValue& value, ZephyrValueMap& localVariables )
{
	if ( value.GetType() != eValueType::NUMBER )
	{
		ReportError( Stringf( "Cannot assign a value of type '%s' to Vec2 variable '%s' member '%s'", ToString( value.GetType() ).c_str(), variableName.c_str(), memberName.c_str() ) );
		return;
	}

	// Try to find in local variables first
	auto localIter = localVariables.find( variableName );
	if ( localIter != localVariables.end() )
	{
		Vec2 vecValue = localIter->second.GetAsVec2();
		if		( memberName == "x" ) { vecValue.x = value.GetAsNumber(); }
		else if ( memberName == "y" ) { vecValue.y = value.GetAsNumber(); }

		localVariables[variableName] = ZephyrValue( vecValue );
		return;
	}

	// Check state variables
	if ( m_stateVariables != nullptr )
	{
		auto stateIter = m_stateVariables->find( variableName );
		if ( stateIter != m_stateVariables->end() )
		{
			Vec2 vecValue = stateIter->second.GetAsVec2();
			if		( memberName == "x" ) { vecValue.x = value.GetAsNumber(); }
			else if ( memberName == "y" ) { vecValue.y = value.GetAsNumber(); }

			( *m_stateVariables )[variableName] = ZephyrValue( vecValue );
			return;
		}
	}

	// Check global variables
	if ( m_globalVariables != nullptr )
	{
		auto globalIter = m_globalVariables->find( variableName );
		if ( globalIter != m_globalVariables->end() )
		{
			Vec2 vecValue = globalIter->second.GetAsVec2();
			if		( memberName == "x" ) { vecValue.x = value.GetAsNumber(); }
			else if ( memberName == "y" ) { vecValue.y = value.GetAsNumber(); }

			( *m_globalVariables )[variableName] = ZephyrValue( vecValue );
		}
	}
}


//-----------------------------------------------------------------------------------------------
// ZEPHYR TYPE TODO: Remove/codegen
void ZephyrVirtualMachine::AssignToVec3MemberVariable( const std::string& variableName, const std::string& memberName, const ZephyrValue& value, ZephyrValueMap& localVariables )
{
	if ( value.GetType() != eValueType::NUMBER )
	{
		ReportError( Stringf( "Cannot assign a value of type '%s' to Vec3 variable '%s' member '%s'", ToString( value.GetType() ).c_str(), variableName.c_str(), memberName.c_str() ) );
		return;
	}

	// Try to find in local variables first
	auto localIter = localVariables.find( variableName );
	if ( localIter != localVariables.end() )
	{
		Vec3 vecValue = localIter->second.GetAsVec3();
		if		( memberName == "x" ) { vecValue.x = value.GetAsNumber(); }
		else if ( memberName == "y" ) { vecValue.y = value.GetAsNumber(); }
		else if ( memberName == "z" ) { vecValue.z = value.GetAsNumber(); }

		localVariables[variableName] = ZephyrValue( vecValue );
		return;
	}

	// Check state variables
	if ( m_stateVariables != nullptr )
	{
		auto stateIter = m_stateVariables->find( variableName );
		if ( stateIter != m_stateVariables->end() )
		{
			Vec3 vecValue = stateIter->second.GetAsVec3();
			if		( memberName == "x" ) { vecValue.x = value.GetAsNumber(); }
			else if ( memberName == "y" ) { vecValue.y = value.GetAsNumber(); }
			else if ( memberName == "z" ) { vecValue.z = value.GetAsNumber(); }

			( *m_stateVariables )[variableName] = ZephyrValue( vecValue );
			return;
		}
	}

	// Check global variables
	if ( m_globalVariables != nullptr )
	{
		auto globalIter = m_globalVariables->find( variableName );
		if ( globalIter != m_globalVariables->end() )
		{
			Vec3 vecValue = globalIter->second.GetAsVec3();
			if		( memberName == "x" ) { vecValue.x = value.GetAsNumber(); }
			else if ( memberName == "y" ) { vecValue.y = value.GetAsNumber(); }
			else if ( memberName == "z" ) { vecValue.z = value.GetAsNumber(); }

			( *m_globalVariables )[variableName] = ZephyrValue( vecValue );
		}
	}
}


//-----------------------------------------------------------------------------------------------
MemberAccessorResult ZephyrVirtualMachine::ProcessResultOfMemberAccessor( const ZephyrValueMap& localVariables )
{
	MemberAccessorResult memberAccessResult;

	ZephyrValue memberCountZephyr = PopConstant();
	int memberCount = (int)memberCountZephyr.GetAsNumber();

	ZephyrValue baseObjName = PopConstant();

	// Pull all members from the constant stack into a temp buffer for processing 
	// so in case there is a member error the stack is left in a good(ish) state 
	// Note: the members will be in reverse order, so account for that
	std::vector<std::string> memberNames;
	memberNames.resize( memberCount );
	for ( int memberIdx = memberCount - 1; memberIdx >= 0; --memberIdx )
	{
		memberNames[memberIdx] = PopConstant().GetAsString();
	}

	// Find base object in this bytecode chunk
	ZephyrValue memberVal = GetVariableValue( baseObjName.GetAsString(), localVariables );
	if ( IsErrorValue( memberVal ) )
	{
		return memberAccessResult;
	}

	std::vector<EntityId> entityIdChain;

	// Process accessors excluding the final component, since that needs to be handled separately
	for ( int memberNameIdx = 0; memberNameIdx < (int)memberNames.size() - 1; ++memberNameIdx )
	{
		std::string& memberName = memberNames[memberNameIdx];

		switch ( memberVal.GetType() )
		{
			// This isn't the last member, so it can't be a primitive type
			case eValueType::BOOL:
			case eValueType::NUMBER:
			case eValueType::STRING:
			{
				ReportError( Stringf( "Variable of type %s can't have members. Tried to access '%s'",
									  ToString( memberVal.GetType() ).c_str(),
									  memberName.c_str() ) );
				return memberAccessResult;
			}
			break;

			case eValueType::VEC2:
			{
				if		( memberName == "x" ) { memberVal = ZephyrValue( memberVal.GetAsVec2().x ); }
				else if ( memberName == "y" ) { memberVal = ZephyrValue( memberVal.GetAsVec2().y ); }
				else
				{
					ReportError( Stringf( "'%s' is not a member of Vec2", memberName.c_str() ) );
					return memberAccessResult;
				}
			}
			break;

			case eValueType::VEC3:
			{
				if		( memberName == "x" ) { memberVal = ZephyrValue( memberVal.GetAsVec3().x ); }
				else if ( memberName == "y" ) { memberVal = ZephyrValue( memberVal.GetAsVec3().y ); }
				else if ( memberName == "z" ) { memberVal = ZephyrValue( memberVal.GetAsVec3().y ); }
				else
				{
					ReportError( Stringf( "'%s' is not a member of Vec3", memberName.c_str() ) );
					return memberAccessResult;
				}
			}
			break;

			case eValueType::ENTITY:
			{
				ZephyrValue val = GetGlobalVariableFromEntity( memberVal.GetAsEntity(), memberName );
				if ( IsErrorValue( val ) )
				{
					std::string entityVarName = memberNameIdx > 0 ? memberNames[memberNameIdx - 1] : baseObjName.GetAsString();

					ReportError( Stringf( "Variable '%s' is not a member of Entity '%s'", memberName.c_str(), entityVarName.c_str() ) );
					return memberAccessResult;
				}

				entityIdChain.push_back( memberVal.GetAsEntity() );
				memberVal = val;
			}
			break;

			case eValueType::USER_TYPE:
			{
				ZephyrTypeBase* userTypeObj = memberVal.GetAsUserType();
				if ( userTypeObj == nullptr )
				{
					ReportError( Stringf( "Variable '%s' is null, cannot dereference it", memberName.c_str() ) );
					return memberAccessResult;
				}

			/*	if ( !userTypeObj->DoesTypeHaveMemberVariable( memberName )
					 && !userTypeObj->DoesTypeHaveMethod( memberName ) )
				{
					ReportError( Stringf( "Member '%s' does not exist in object of type '%s'", memberName.c_str(), userTypeObj->GetTypeName().c_str() ) );
					return memberAccessResult;
				}*/

				// TODO: Remove when refactoring Vec2 and Vec3, used to set members in other entities
				//entityIdChain.push_back( memberVal.GetAsEntity() );
				memberVal = ZephyrValue( userTypeObj );
			}
			break;
		}
	}

	memberAccessResult.finalMemberVal = memberVal;
	memberAccessResult.baseObjName = baseObjName.GetAsString();
	memberAccessResult.memberNames = memberNames;
	memberAccessResult.entityIdChain = entityIdChain;

	return memberAccessResult;
}


//-----------------------------------------------------------------------------------------------
std::map<std::string, std::string> ZephyrVirtualMachine::GetCallerVariableToParamNamesFromParameters( const std::string& eventName )
{
	std::map<std::string, std::string> identifierToParamNames;

	ZephyrValue identifierCount = PopConstant();
	for ( int identifierIdx = 0; identifierIdx < (int)identifierCount.GetAsNumber(); ++identifierIdx )
	{
		ZephyrValue identifierParamName = PopConstant();
		ZephyrValue identifier = PopConstant();
		if ( identifier.GetType() != eValueType::STRING
			 || identifierParamName.GetType() != eValueType::STRING )
		{
			ReportError( Stringf( "Identifier name is not a string in parameter list for function call '%s'", eventName.c_str() ) );
			continue;
		}
		identifierToParamNames[identifier.GetAsString()] = identifierParamName.GetAsString();
	}

	return identifierToParamNames;
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::InsertParametersIntoEventArgs( ZephyrArgs& args )
{
	ZephyrValue paramCount = PopConstant();

	for ( int paramIdx = 0; paramIdx < (int)paramCount.GetAsNumber(); ++paramIdx )
	{
		ZephyrValue param = PopConstant();
		ZephyrValue value = PopConstant();

		switch ( value.GetType() )
		{
			case eValueType::BOOL:		args.SetValue( param.GetAsString(), value.GetAsBool() ); break;
			case eValueType::NUMBER:	args.SetValue( param.GetAsString(), value.GetAsNumber() ); break;
			case eValueType::VEC2:		args.SetValue( param.GetAsString(), value.GetAsVec2() ); break;
			case eValueType::VEC3:		args.SetValue( param.GetAsString(), value.GetAsVec3() ); break;
			case eValueType::STRING:	args.SetValue( param.GetAsString(), value.GetAsString() ); break;
			case eValueType::ENTITY:	args.SetValue( param.GetAsString(), value.GetAsEntity() ); break;
			case eValueType::USER_TYPE:	args.SetValue( param.GetAsString(), (ZephyrTypeBase*)value.EvaluateAsUserType() ); break;
			default: ERROR_AND_DIE( Stringf( "Unimplemented event arg type '%s'", ToString( value.GetType() ).c_str() ) );
		}
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::UpdateIdentifierParameters( const std::map<std::string, std::string>& identifierToParamNames, const ZephyrArgs& args, ZephyrValueMap& localVariables )
{
	for ( auto const& identifierPair : identifierToParamNames )
	{
		std::string const& identifier = identifierPair.first;
		std::string const& paramName = identifierPair.second;

		ZephyrValue newVal = GetZephyrValFromEventArgs( paramName, args );
		
		if ( newVal.GetType() == eValueType::ENTITY 
			 && newVal == ZephyrValue::ERROR_VALUE )
		{
			//ReportError( Stringf( "No matching arg found for parameter '%s'", param.c_str() ) );
			continue;
		}
		
		AssignToVariable( identifier, newVal, localVariables );
	}
}


//-----------------------------------------------------------------------------------------------
ZephyrValue ZephyrVirtualMachine::GetZephyrValFromEventArgs( const std::string& varName, const ZephyrArgs& args )
{
	auto const& keyValuePairs = args.GetAllKeyValuePairs();
	auto iter = keyValuePairs.find( varName );
	if ( iter == keyValuePairs.end() )
	{
		return ZephyrValue::ERROR_VALUE;
	}
	
	if ( iter->second->Is<float>() )
	{
		ZephyrArgs params;
		params.SetValue( "value", args.GetValue( varName, 0.f ) );
		return ZephyrValue( g_zephyrTypeObjFactory->CreateObject( "Number", &params ) );
		//return ZephyrValue( args.GetValue( varName, 0.f ) );
	}
	else if ( iter->second->Is<EntityId>() )
	{
		return ZephyrValue( args.GetValue( varName, (EntityId)ERROR_ZEPHYR_ENTITY_ID ) );
	}
	else if ( iter->second->Is<double>() )
	{
		return ZephyrValue( (float)args.GetValue( varName, 0.0 ) );
	}
	else if ( iter->second->Is<bool>() )
	{
		return ZephyrValue( args.GetValue( varName, false ) );
	}
	// ZEPHYR TYPE TODO: Remove/codegen
	else if ( iter->second->Is<Vec2>() )
	{
		return ZephyrValue( args.GetValue( varName, Vec2::ZERO ) );
	}
	else if ( iter->second->Is<Vec3>() )
	{
		return ZephyrValue( args.GetValue( varName, Vec3::ZERO ) );
	}
	else if ( iter->second->Is<std::string>()
			  || iter->second->Is<char*>() )
	{
		return ZephyrValue( iter->second->GetAsString() );
	}
	else if ( iter->second->Is<ZephyrTypeBase*>() )
	{
		return ZephyrValue( args.GetValue( varName, (ZephyrTypeBase*)nullptr ) );
		//return ZephyrValue( (ZephyrTypeBase*)iter->second );
		//ReportError( "ZephyrValue saved into args but wasn't user type" );
	}

	return ZephyrValue::ERROR_VALUE;
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
// ZEPHYR TYPE TODO: Remove/codegen
void ZephyrVirtualMachine::SetGlobalVec2MemberVariableInEntity( EntityId entityId, const std::string& variableName, const std::string& memberName, const ZephyrValue& value )
{
	Entity* entity = g_zephyrAPI->GetEntityById( entityId );
	if ( entity == nullptr )
	{
		ReportError( Stringf( "Unknown entity does not contain a member '%s'", variableName.c_str() ) );
		return;
	}
	ZephyrValue oldValue = ZephyrSystem::GetGlobalVariable( entityId, variableName );

	Vec2 newValue = oldValue.GetAsVec2();
	if ( memberName == "x" )
	{
		newValue.x = value.GetAsNumber();
	}
	if ( memberName == "y" )
	{
		newValue.y = value.GetAsNumber();
	}

	return ZephyrSystem::SetGlobalVariable( entityId, variableName, ZephyrValue( newValue ) );
}


//-----------------------------------------------------------------------------------------------
// ZEPHYR TYPE TODO: Remove/codegen
void ZephyrVirtualMachine::SetGlobalVec3MemberVariableInEntity( EntityId entityId, const std::string& variableName, const std::string& memberName, const ZephyrValue& value )
{
	Entity* entity = g_zephyrAPI->GetEntityById( entityId );
	if ( entity == nullptr )
	{
		ReportError( Stringf( "Unknown entity does not contain a member '%s'", variableName.c_str() ) );
		return;
	}
	ZephyrValue oldValue = ZephyrSystem::GetGlobalVariable( entityId, variableName );

	Vec3 newValue = oldValue.GetAsVec3();
	if ( memberName == "x" )
	{
		newValue.x = value.GetAsNumber();
	}
	if ( memberName == "y" )
	{
		newValue.y = value.GetAsNumber();
	}
	if ( memberName == "z" )
	{
		newValue.z = value.GetAsNumber();
	}

	return ZephyrSystem::SetGlobalVariable( entityId, variableName, ZephyrValue( newValue ) );
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
bool ZephyrVirtualMachine::CallMemberFunctionOnUserType( ZephyrTypeBase& userObj, const std::string& functionName, ZephyrArgs* args )
{
	userObj.CallMethod( functionName, args );
	
	return true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrVirtualMachine::ReportError( const std::string& errorMsg )
{
	g_devConsole->PrintError( Stringf( "Error in script '%s': %s", m_zephyrComponent.GetScriptName().c_str(), errorMsg.c_str() ) );

	m_zephyrComponent.m_compState = eComponentState::INVALID_SCRIPT;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrVirtualMachine::IsErrorValue( const ZephyrValue& zephyrValue )
{
	return zephyrValue.GetAsEntity() == ZephyrValue::ERROR_VALUE;
}
