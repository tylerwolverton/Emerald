#include "Engine/Zephyr/Core/ZephyrCommon.hpp"
#include "Engine/Core/DevConsole.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/NamedProperties.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Time/Clock.hpp"

#include "Engine/Zephyr/Core/ZephyrUtils.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"


//-----------------------------------------------------------------------------------------------
std::string PARENT_ENTITY_STR = "parentEntity";
std::string PARENT_ENTITY_NAME_STR = "parentEntityName";
std::string TARGET_ENTITY_STR = "targetEntity";
std::string TARGET_ENTITY_NAME_STR = "targetName";

ZephyrEngineEvents* g_zephyrAPI = nullptr;
ZephyrSubsystem* g_zephyrSubsystem = nullptr;
ZephyrTypeHandleFactory* g_zephyrTypeHandleFactory = nullptr;

ZephyrHandle NULL_ZEPHYR_HANDLE = ZephyrHandle();
const ZephyrValue ZephyrValue::ERROR_VALUE = ZephyrValue( (EntityId)ERROR_ZEPHYR_ENTITY_ID );


//-----------------------------------------------------------------------------------------------
std::string ToString( eTokenType type )
{
	switch ( type )
	{
		case eTokenType::BRACE_LEFT:		return "{";
		case eTokenType::BRACE_RIGHT:		return "}";
		case eTokenType::PARENTHESIS_LEFT:	return "(";
		case eTokenType::PARENTHESIS_RIGHT:	return ")";
		case eTokenType::STATE:				return "State";
		case eTokenType::FUNCTION:			return "Function";
		case eTokenType::ENTITY:			return "Entity";
		case eTokenType::USER_TYPE:			return "UserType";
		case eTokenType::ON_ENTER:			return "OnEnter";
		case eTokenType::ON_UPDATE:			return "OnUpdate";
		case eTokenType::ON_EXIT:			return "OnExit";
		case eTokenType::CHANGE_STATE:		return "ChangeState";
		case eTokenType::IF:				return "if";
		case eTokenType::ELSE:				return "else";
		case eTokenType::RETURN:			return "return";
		case eTokenType::TRUE_TOKEN:		return "true";
		case eTokenType::FALSE_TOKEN:		return "false";
		case eTokenType::NULL_TOKEN:		return "null";
		case eTokenType::IDENTIFIER:		return "Identifier";
		case eTokenType::CONSTANT_NUMBER:	return "Constant Number";
		case eTokenType::CONSTANT_STRING:	return "Constant String";
		case eTokenType::PLUS:				return "+";
		case eTokenType::MINUS:				return "-";
		case eTokenType::STAR:				return "*";
		case eTokenType::SLASH:				return "/";
		case eTokenType::AND:				return "&&";
		case eTokenType::OR:				return "||";
		case eTokenType::EQUAL:				return "=";
		case eTokenType::EQUAL_EQUAL:		return "==";
		case eTokenType::BANG:				return "!";
		case eTokenType::BANG_EQUAL:		return "!=";
		case eTokenType::GREATER:			return ">";
		case eTokenType::GREATER_EQUAL:		return ">=";
		case eTokenType::LESS:				return "<";
		case eTokenType::LESS_EQUAL:		return "<=";
		case eTokenType::SEMICOLON:			return ";";
		case eTokenType::COLON:				return ":";
		case eTokenType::COMMA:				return ",";
		case eTokenType::QUOTE:				return "\"";
		case eTokenType::PERIOD:			return ".";
		case eTokenType::END_OF_FILE:		return "End of File";
		default:							return "Unknown";
	}
}


//-----------------------------------------------------------------------------------------------
std::string GetTokenName( eTokenType type )
{
	switch ( type )
	{
		case eTokenType::BRACE_LEFT:		return "BRACE_LEFT";
		case eTokenType::BRACE_RIGHT:		return "BRACE_RIGHT";
		case eTokenType::PARENTHESIS_LEFT:	return "PARENTHESIS_LEFT";
		case eTokenType::PARENTHESIS_RIGHT:	return "PARENTHESIS_RIGHT";
		case eTokenType::STATE:				return "STATE";
		case eTokenType::FUNCTION:			return "FUNCTION";
		case eTokenType::ENTITY:			return "ENTITY";
		case eTokenType::USER_TYPE:			return "USER_TYPE";
		case eTokenType::ON_ENTER:			return "ON_ENTER";
		case eTokenType::ON_UPDATE:			return "ON_UPDATE";
		case eTokenType::ON_EXIT:			return "ON_EXIT";
		case eTokenType::CHANGE_STATE:		return "CHANGE_STATE";
		case eTokenType::IF:				return "IF";
		case eTokenType::ELSE:				return "ELSE";
		case eTokenType::RETURN:			return "RETURN";
		case eTokenType::TRUE_TOKEN:		return "TRUE";
		case eTokenType::FALSE_TOKEN:		return "FALSE";
		case eTokenType::NULL_TOKEN:		return "NULL_TOKEN";
		case eTokenType::IDENTIFIER:		return "IDENTIFIER";
		case eTokenType::CONSTANT_NUMBER:	return "CONSTANT_NUMBER";
		case eTokenType::CONSTANT_STRING:	return "CONSTANT_STRING";
		case eTokenType::PLUS:				return "PLUS";
		case eTokenType::MINUS:				return "MINUS";
		case eTokenType::STAR:				return "STAR";
		case eTokenType::SLASH:				return "SLASH";
		case eTokenType::AND:				return "AND";
		case eTokenType::OR:				return "OR";
		case eTokenType::EQUAL:				return "EQUAL";
		case eTokenType::EQUAL_EQUAL:		return "EQUAL_EQUAL";
		case eTokenType::BANG:				return "BANG";
		case eTokenType::BANG_EQUAL:		return "BANG_EQUAL";
		case eTokenType::GREATER:			return "GREATER";
		case eTokenType::GREATER_EQUAL:		return "GREATER_EQUAL";
		case eTokenType::LESS:				return "LESS";
		case eTokenType::LESS_EQUAL:		return "LESS_EQUAL";
		case eTokenType::SEMICOLON:			return "SEMICOLON";
		case eTokenType::COLON:				return "COLON";
		case eTokenType::COMMA: 			return "COMMA";
		case eTokenType::QUOTE: 			return "QUOTE";
		case eTokenType::PERIOD: 			return "PERIOD";
		case eTokenType::END_OF_FILE:		return "END_OF_FILE";
		default:							return "UNKNOWN";
	}
}


//-----------------------------------------------------------------------------------------------
eOpCode ByteToOpCode( byte opCodeByte )
{
	if ( opCodeByte < 0
			|| opCodeByte >= (byte)eOpCode::LAST_VAL )
	{
		return eOpCode::UNKNOWN;
	}

	return (eOpCode)opCodeByte;	
}


//-----------------------------------------------------------------------------------------------
std::string ToString( eOpCode opCode )
{
	switch ( opCode )
	{
		case eOpCode::NEGATE:					return "NEGATE";
		case eOpCode::NOT:						return "NOT";
		case eOpCode::CONSTANT:					return "CONSTANT";
		case eOpCode::CONSTANT_USER_TYPE:		return "CONSTANT_USER_TYPE";
		case eOpCode::GET_VARIABLE_VALUE:		return "GET_VARIABLE_VALUE";
		case eOpCode::ASSIGNMENT:				return "ASSIGNMENT";
		case eOpCode::MEMBER_ASSIGNMENT:		return "MEMBER_ASSIGNMENT";
		case eOpCode::MEMBER_ACCESSOR:			return "MEMBER_ACCESSOR";
		case eOpCode::MEMBER_FUNCTION_CALL:		return "MEMBER_FUNCTION_CALL";
		case eOpCode::ADD:						return "ADD";
		case eOpCode::SUBTRACT:					return "SUBTRACT";
		case eOpCode::MULTIPLY:					return "MULTIPLY";
		case eOpCode::DIVIDE:					return "DIVIDE";
		case eOpCode::NOT_EQUAL:				return "NOT_EQUAL";
		case eOpCode::EQUAL:					return "EQUAL";
		case eOpCode::GREATER:					return "GREATER";
		case eOpCode::GREATER_EQUAL:			return "GREATER_EQUAL";
		case eOpCode::LESS:						return "LESS";
		case eOpCode::LESS_EQUAL:				return "LESS_EQUAL";
		case eOpCode::FUNCTION_CALL:			return "FUNCTION_CALL";
		case eOpCode::CHANGE_STATE:				return "CHANGE_STATE";
		case eOpCode::RETURN:					return "RETURN";
		case eOpCode::IF:						return "IF";
		case eOpCode::JUMP:						return "JUMP";
		case eOpCode::AND:						return "AND";
		case eOpCode::OR:						return "OR";
		case eOpCode::SET_SCOPE:				return "SET_SCOPE";
		case eOpCode::LAST_VAL:					return "LAST_VAL";
		default:								return "UNKNOWN";
	}
}

//-----------------------------------------------------------------------------------------------
std::string ToString( eValueType valueType )
{
	switch ( valueType )
	{
		case eValueType::NUMBER:	return "Number";
		case eValueType::BOOL:		return "Bool";
		case eValueType::STRING:	return "String";
		case eValueType::ENTITY:	return "Entity";
		case eValueType::USER_TYPE:	return "UserType";

		case eValueType::NONE:		
		default: return "None";
	}
}


//-----------------------------------------------------------------------------------------------
eValueType FromString( const std::string& strType )
{
	if ( strType == "Number" )		{ return eValueType::NUMBER; }
	if ( strType == "Bool" )		{ return eValueType::BOOL; }	
	if ( strType == "String" )		{ return eValueType::STRING; }
	if ( strType == "Entity" )		{ return eValueType::ENTITY; }
	if ( strType == "UserType" )	{ return eValueType::USER_TYPE; }

	return eValueType::NONE;
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeMetadata::ZephyrTypeMetadata( const std::string& typeName )
	: m_typeName( typeName )
{
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeMethod* ZephyrTypeMetadata::GetMethod( const std::string& methodName )
{
	for ( ZephyrTypeMethod& registeredMethod : m_methods )
	{
		if ( methodName == registeredMethod.name )
		{
			return &registeredMethod;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeMemberVariable* ZephyrTypeMetadata::GetMemberVariable( const std::string& memberName )
{
	for ( ZephyrTypeMemberVariable& registeredMember : m_memberVariables )
	{
		if ( memberName == registeredMember.name )
		{
			return &registeredMember;
		}
	}

	return nullptr;
}


//-----------------------------------------------------------------------------------------------
void ZephyrTypeMetadata::RegisterMember( const std::string& memberName, const std::string& memberType )
{
	m_memberVariables.emplace_back( memberName, memberType );
}


//-----------------------------------------------------------------------------------------------
void ZephyrTypeMetadata::RegisterReadOnlyMember( const std::string& memberName, const std::string& memberType )
{
	m_memberVariables.emplace_back( memberName, memberType );
	m_memberVariables.back().isReadonly = true;
}


//-----------------------------------------------------------------------------------------------
void ZephyrTypeMetadata::RegisterMethod( const std::string& methodName )
{
	m_methods.emplace_back( methodName );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrTypeMetadata::HasMemberVariable( const std::string& varName )
{
	for ( const ZephyrTypeMemberVariable& member : m_memberVariables )
	{
		if ( varName == member.name )
		{
			return true;
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrTypeMetadata::HasMethod( const std::string& methodName )
{
	return GetMethod( methodName ) != nullptr;
}


//-----------------------------------------------------------------------------------------------
ZephyrTypeBase::ZephyrTypeBase( const std::string& typeName )
{
	m_typeMetadata = g_zephyrSubsystem->GetCopyOfRegisteredUserType( typeName );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrTypeBase::HasMemberVariable( const std::string& varName )
{
	return m_typeMetadata.HasMethod( varName );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrTypeBase::HasMethod( const std::string& methodName )
{
	return m_typeMetadata.HasMethod( methodName );
}


//-----------------------------------------------------------------------------------------------
void ZephyrTypeBase::CallMethod( const std::string& methodName, ZephyrArgs* args )
{
	ZephyrTypeMethod* methodToCall = m_typeMetadata.GetMethod( methodName );
	if ( methodToCall == nullptr )
	{
		// Error
		return;
	}

	methodToCall->delegate.Invoke( args );
}


//-----------------------------------------------------------------------------------------------
eZephyrComparatorResult ZephyrTypeBase::NotEqual(ZephyrHandle other)
{
	eZephyrComparatorResult equalResult = Equal(other);
	if ( equalResult == eZephyrComparatorResult::FALSE_VAL )		{ return eZephyrComparatorResult::TRUE_VAL; }
	if ( equalResult == eZephyrComparatorResult::TRUE_VAL )			{ return eZephyrComparatorResult::FALSE_VAL; }

	return eZephyrComparatorResult::UNDEFINED_VAL;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue::ZephyrValue()
{
	m_type = eValueType::NONE;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue::ZephyrValue( NUMBER_TYPE value )
{
	m_type = eValueType::NUMBER;
	numberData = value;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue::ZephyrValue( bool value )
{
	m_type = eValueType::BOOL;
	boolData = value;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue::ZephyrValue( const std::string& value )
{
	m_type = eValueType::STRING;
	strData = new std::string( value );
}


//-----------------------------------------------------------------------------------------------
ZephyrValue::ZephyrValue( EntityId value )
{
	m_type = eValueType::ENTITY;
	entityData = value;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue::ZephyrValue( ZephyrHandle value )
{
	m_type = eValueType::USER_TYPE;
	userTypeData = value;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue::ZephyrValue( ZephyrValue const& other )
{
	// If this is already a string type delete the data before setting new data
	if ( this->m_type == eValueType::STRING )
	{
		delete this->strData;
		this->strData = nullptr;
	}

	switch ( other.m_type )
	{
		case eValueType::STRING:	this->strData = new std::string( *other.strData );	break;
		case eValueType::NUMBER:	this->numberData = other.numberData;	break;
		case eValueType::BOOL:		this->boolData = other.boolData;	break;
		case eValueType::ENTITY:	this->entityData = other.entityData;	break;
		case eValueType::USER_TYPE:	this->userTypeData = other.userTypeData;	break;
	}

	m_type = other.m_type;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue& ZephyrValue::operator=( ZephyrValue const& other )
{
	if ( this->m_type == eValueType::USER_TYPE && other.GetType() == eValueType::USER_TYPE )
	{
		//if ( this->userTypeData->GetTypeName() != other.GetAsUserType()->GetTypeName() )
		//{
		//	// Trying to set a user type to a different type
		//}

		this->userTypeData = other.GetAsUserType();

		return *this;
	}

	if ( this->m_type == eValueType::STRING )
	{
		delete this->strData;
		this->strData = nullptr;
	}

	switch ( other.m_type )
	{
		case eValueType::STRING:	this->strData = new std::string( *other.strData );	break;
		case eValueType::NUMBER:	this->numberData = other.numberData;	break;
		case eValueType::BOOL:		this->boolData = other.boolData;	break;
		case eValueType::ENTITY:	this->entityData = other.entityData;	break;
		case eValueType::USER_TYPE:	this->userTypeData = other.userTypeData;	break;
	}

	m_type = other.m_type;

	return *this;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue::ZephyrValue( ZephyrValue const&& other )
{
	// If this is already a string type delete the data before setting new data
	if ( this->m_type == eValueType::STRING  )
	{
		delete this->strData;
		this->strData = nullptr;
	}

	switch ( other.m_type )
	{
		case eValueType::STRING:	this->strData = new std::string( *other.strData );	break;
		case eValueType::NUMBER:	this->numberData = other.numberData;	break;
		case eValueType::BOOL:		this->boolData = other.boolData;	break;
		case eValueType::ENTITY:	this->entityData = other.entityData;	break;
		case eValueType::USER_TYPE:	this->userTypeData = other.userTypeData;	break;
	}

	m_type = other.m_type;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue& ZephyrValue::operator=( ZephyrValue const&& other )
{
	if ( this == &other )
	{
		return *this;
	}

	if ( this->m_type == eValueType::STRING )
	{
		delete this->strData;
		this->strData = nullptr;
	}

	switch ( other.m_type )
	{
		case eValueType::STRING:	this->strData = new std::string( *other.strData );	break;
		case eValueType::NUMBER:	this->numberData = other.numberData;	break;
		case eValueType::BOOL:		this->boolData = other.boolData;	break;
		case eValueType::ENTITY:	this->entityData = other.entityData;	break;
		case eValueType::USER_TYPE:	this->userTypeData = other.userTypeData;	break;
	}

	m_type = other.m_type;

	return *this;
}


//-----------------------------------------------------------------------------------------------
ZephyrValue::~ZephyrValue()
{
	if ( m_type == eValueType::STRING )
	{
		delete strData;
		strData = nullptr;
	}

	if ( m_type == eValueType::USER_TYPE )
	{
		userTypeData.~Handle();
	}
}


//-----------------------------------------------------------------------------------------------
std::string ZephyrValue::GetAsString() const
{
	if ( strData == nullptr )
	{
		return "";
	}

	return *strData;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrValue::EvaluateAsBool()
{
	switch ( m_type )
	{
		case eValueType::STRING: 	return !strData->empty();	
		case eValueType::NUMBER: 	return !IsNearlyEqual( numberData, 0.f );			
		case eValueType::BOOL:		return boolData;	
		case eValueType::ENTITY:	return entityData != INVALID_ENTITY_ID;
		case eValueType::USER_TYPE:	
		{
			if ( userTypeData.IsValid() )
			{
				SmartPtr<ZephyrTypeBase> smartPtr( userTypeData );
				return smartPtr->EvaluateAsBool();
			}
			else
			{
				return false;
			}
		}
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
std::string ZephyrValue::EvaluateAsString()
{
	switch ( m_type )
	{
		case eValueType::STRING: 	return GetAsString();
		case eValueType::NUMBER: 	return ToString( numberData );
		case eValueType::BOOL:		return ToString( boolData );
		case eValueType::ENTITY:	return ToString( entityData );
		case eValueType::USER_TYPE:	
		{
			if ( userTypeData.IsValid() )
			{
				SmartPtr<ZephyrTypeBase> smartPtr( userTypeData );
				smartPtr->ToString();
			}
			else
			{
				return "";
			}
		}
	}

	return "";
}


//-----------------------------------------------------------------------------------------------
float ZephyrValue::EvaluateAsNumber()
{
	switch ( m_type )
	{
		case eValueType::NUMBER: 	return numberData;
		case eValueType::BOOL:		return boolData ? 1.f : 0.f;
		default:	
			ReportConversionError( eValueType::NUMBER );
	}

	return (float)ERROR_ZEPHYR_ENTITY_ID;
}


//-----------------------------------------------------------------------------------------------
EntityId ZephyrValue::EvaluateAsEntity()
{
	if ( m_type == eValueType::ENTITY )
	{
		return entityData;
	}
		
	ReportConversionError( eValueType::ENTITY );
	return ERROR_ZEPHYR_ENTITY_ID;
}


//-----------------------------------------------------------------------------------------------
ZephyrHandle ZephyrValue::EvaluateAsUserType()
{
	if ( m_type == eValueType::USER_TYPE )
	{
		return userTypeData;
	}

	ReportConversionError( eValueType::USER_TYPE );
	return NULL_ZEPHYR_HANDLE;
}


//-----------------------------------------------------------------------------------------------
std::string ZephyrValue::SerializeToString() const
{
	std::string serializedStr = ToString( m_type );
	serializedStr += ":";
	switch ( m_type )
	{
		case eValueType::STRING: 	serializedStr += GetAsString();
		case eValueType::NUMBER: 	serializedStr += ToString( numberData );
		case eValueType::BOOL:		serializedStr += ToString( boolData );
		case eValueType::ENTITY:	serializedStr += ToString( entityData );
		case eValueType::USER_TYPE:	
		{		
			ZephyrHandle userHandle = GetAsUserType();
			if ( userHandle.IsValid() )
			{
				SmartPtr userPtr( userHandle );
				serializedStr += userPtr->ToString();
			}
		}
	}

	return serializedStr;
}


//-----------------------------------------------------------------------------------------------
void ZephyrValue::DeserializeFromString( const std::string& serlializedStr )
{
	// This will destroy the ZephyrValue even if the serialization fails
	if ( this->m_type == eValueType::STRING )
	{
		delete this->strData;
		this->strData = nullptr;
	}

	// Split serialized string into type and data
	Strings components = SplitStringOnDelimiter( serlializedStr, ':' );
	m_type = FromString(components[0]);
	const char* dataStr = components[1].c_str();

	switch ( m_type )
	{
		case eValueType::STRING: 	{ strData = new std::string( dataStr ); }
		case eValueType::NUMBER:	{ numberData = FromString( dataStr, 0.f ); }
		case eValueType::BOOL:		{ boolData = FromString( dataStr, false ); }
		case eValueType::ENTITY:	{ entityData = (EntityId)FromString( dataStr, (EntityId)-1 ); }
		//case eValueType::USER_TYPE: { userTypeData->FromString( dataStr ); }
	}
}


//-----------------------------------------------------------------------------------------------
void ZephyrValue::ReportConversionError( eValueType targetType )
{
	g_devConsole->PrintError( Stringf( "Cannot access '%s' variable as type '%s'", ToString( m_type ).c_str(), ToString( targetType ).c_str() ) );
	entityData = ERROR_ZEPHYR_ENTITY_ID;
}


//-----------------------------------------------------------------------------------------------
bool operator==( const ZephyrValue& lhs, const ZephyrValue& rhs )
{
	if ( lhs.GetType() != rhs.GetType() )
	{
		return false;
	}

	switch ( lhs.GetType() )
	{
		case eValueType::STRING:	{ return lhs.GetAsString() == rhs.GetAsString(); }
		case eValueType::NUMBER:	{ return lhs.GetAsNumber() == rhs.GetAsNumber(); }
		case eValueType::BOOL:		{ return lhs.GetAsBool() == rhs.GetAsBool(); }
		case eValueType::ENTITY:	{ return lhs.GetAsEntity() == rhs.GetAsEntity(); }
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
ZephyrTimer::ZephyrTimer( Clock* clock )
{
	timer = Timer( clock );
	callbackArgs = new EventArgs();
}


//-----------------------------------------------------------------------------------------------
ZephyrTimer::~ZephyrTimer()
{
	PTR_SAFE_DELETE( callbackArgs );
}


//-----------------------------------------------------------------------------------------------
bool ZephyrScope::TryToGetVariable( const std::string& identifier, ZephyrValue& out_value ) const
{
	ZephyrScope const* curScope = this;
	while ( curScope != nullptr )
	{
		auto variableEntry = curScope->variables.find( identifier );
		if ( variableEntry != curScope->variables.end() )
		{
			out_value = variableEntry->second;
			return true;
		}

		curScope = curScope->parentScope;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrScope::SetVariable( const std::string& identifier, const ZephyrValue& value )
{
	ZephyrScope* curScope = this;
	while ( curScope != nullptr )
	{
		auto variableEntry = curScope->variables.find( identifier );
		if ( variableEntry != curScope->variables.end() )
		{
			curScope->variables[identifier] = value;
			return true;
		}

		curScope = curScope->parentScope;
	}

	return false;
}


//-----------------------------------------------------------------------------------------------
bool ZephyrScope::DefineVariable( const std::string& identifier, const ZephyrValue& value )
{
	auto variableEntry = variables.find( identifier );
	if ( variableEntry != variables.end() )
	{
		return false;
	}

	variables[identifier] = value;
	return true;
}

