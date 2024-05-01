#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Memory/HandleFactory.hpp"
#include "Engine/Time/Timer.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"

#include <functional>
#include <map>
#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrValue;
class ZephyrTypeBase;
class ZephyrBytecodeChunk;
class ZephyrComponent;
class ZephyrEngineEvents;
class ZephyrSubsystem;


//-----------------------------------------------------------------------------------------------
#define NUMBER_TYPE float

typedef std::map<std::string, ZephyrValue> ZephyrValueMap;
typedef std::map<std::string, ZephyrBytecodeChunk*> ZephyrBytecodeChunkMap;
typedef std::vector<ZephyrComponent*> ZephyrComponentVector;
typedef NamedProperties ZephyrArgs;
typedef Handle<ZephyrTypeBase> ZephyrHandle;
extern ZephyrHandle NULL_ZEPHYR_HANDLE;

constexpr int ERROR_ZEPHYR_ENTITY_ID = -1000;
extern std::string PARENT_ENTITY_STR;
extern std::string PARENT_ENTITY_NAME_STR;
extern std::string TARGET_ENTITY_STR;
extern std::string TARGET_ENTITY_NAME_STR;

typedef std::string ZephyrTypeId;
typedef ZephyrHandle ( *ZephyrTypeObjCreationFunc )( ZephyrArgs* );
typedef HandleFactory< ZephyrTypeBase, ZephyrTypeId, ZephyrTypeObjCreationFunc, ZephyrArgs* > ZephyrTypeHandleFactory;

extern ZephyrTypeHandleFactory* g_zephyrTypeHandleFactory;
extern ZephyrEngineEvents* g_zephyrAPI;
extern ZephyrSubsystem* g_zephyrSubsystem;


//-----------------------------------------------------------------------------------------------
enum class eTokenType
{
	UNKNOWN,
	ERROR_TOKEN,

	// Keywords
	STATE,
	FUNCTION,
	ENTITY,
	USER_TYPE,
	ON_ENTER,
	ON_UPDATE,
	ON_EXIT,
	CHANGE_STATE,
	IF,
	ELSE,
	RETURN,
	TRUE_TOKEN,
	FALSE_TOKEN,
	NULL_TOKEN,

	BRACE_LEFT,
	BRACE_RIGHT,
	PARENTHESIS_LEFT,
	PARENTHESIS_RIGHT,
	
	IDENTIFIER,

	CONSTANT_NUMBER,
	CONSTANT_STRING,

	// Operators
	PLUS,
	MINUS,
	STAR,
	SLASH,
	EQUAL,
	
	// Logical Operators
	AND,
	OR,

	// Comparators
	EQUAL_EQUAL,
	BANG,
	BANG_EQUAL,
	GREATER,
	GREATER_EQUAL,
	LESS,
	LESS_EQUAL,

	SEMICOLON,
	COLON,
	COMMA,
	QUOTE,
	PERIOD,

	END_OF_FILE,

	LAST_VAL
};

std::string ToString( eTokenType type );
std::string GetTokenName( eTokenType type );


//-----------------------------------------------------------------------------------------------
enum class eOpCode : byte
{
	UNKNOWN,

	NEGATE,
	NOT,

	CONSTANT,
	CONSTANT_USER_TYPE,

	GET_VARIABLE_VALUE,
	ASSIGNMENT,
	MEMBER_ASSIGNMENT,
	MEMBER_ACCESSOR,
	MEMBER_FUNCTION_CALL,

	// Arithmetic
	ADD,
	SUBTRACT, 
	MULTIPLY,
	DIVIDE,

	// Comparators
	NOT_EQUAL,
	EQUAL,
	GREATER,
	GREATER_EQUAL,
	LESS,
	LESS_EQUAL,

	FUNCTION_CALL,
	CHANGE_STATE,

	RETURN,

	IF,
	JUMP,
	AND,
	OR,

	SET_SCOPE,

	LAST_VAL,
};

eOpCode ByteToOpCode( byte opCodeByte );
std::string ToString( eOpCode opCode );

//-----------------------------------------------------------------------------------------------
enum class eValueType
{
	NONE,
	NUMBER,
	BOOL,
	STRING,
	ENTITY,
	USER_TYPE,
};

std::string ToString( eValueType valueType );


//-----------------------------------------------------------------------------------------------
enum class eZephyrComparatorResult
{
	UNDEFINED_VAL = -1,
	FALSE_VAL = 0,
	TRUE_VAL = 1
};


//-----------------------------------------------------------------------------------------------
struct EntityVariableInitializer
{
public:
	std::string varName;
	std::string entityName;

public:
	EntityVariableInitializer( const std::string& varName, const std::string& entityName )
		: varName( varName )
		, entityName( entityName )
	{
	}
};


//-----------------------------------------------------------------------------------------------
struct ZephyrScope
{
public:
	ZephyrScope* parentScope = nullptr;
	ZephyrValueMap variables;

public:
	ZephyrScope() = default;
	ZephyrScope( const ZephyrValueMap& variables, ZephyrScope* parentScope = nullptr )
		: variables( variables )
		, parentScope( parentScope )
	{
	}

	bool TryToGetVariable( const std::string& identifier, ZephyrValue& out_value ) const;
	bool SetVariable( const std::string& identifier, const ZephyrValue& value );
	bool DefineVariable( const std::string& identifier, const ZephyrValue& value );
};


//-----------------------------------------------------------------------------------------------
struct ZephyrTypeMemberVariable
{
public:
	explicit ZephyrTypeMemberVariable( const std::string& memberName, const std::string& memberType )
		: name( memberName )
		, type( memberType )
	{}

	std::string name;
	std::string type;
	bool isReadonly = false;
};


//-----------------------------------------------------------------------------------------------
struct ZephyrTypeMethod
{
public:
	explicit ZephyrTypeMethod( const std::string& methodName )
		: name( methodName )
	{}

	std::string name;
	Delegate<ZephyrArgs*> delegate;
};


//-----------------------------------------------------------------------------------------------
class ZephyrTypeMetadata
{
	friend class ZephyrTypeBase;

public:
	ZephyrTypeMetadata() = default;
	ZephyrTypeMetadata( const std::string& typeName );

	std::string GetTypeName() const									{ return m_typeName; }
	ZephyrTypeMethod* GetMethod( const std::string& methodName );
	ZephyrTypeMemberVariable* GetMemberVariable( const std::string& memberName );

	void RegisterMember( const std::string& memberName, const std::string& memberType );
	void RegisterReadOnlyMember( const std::string& memberName, const std::string& memberType );
	void RegisterMethod( const std::string& methodName );

	bool HasMemberVariable( const std::string& varName );
	bool HasMethod( const std::string& methodName );

private:
	std::string m_typeName;
	std::vector<ZephyrTypeMemberVariable> m_memberVariables;
	std::vector<ZephyrTypeMethod> m_methods;
};


//-----------------------------------------------------------------------------------------------
// The base class for types exposed to Zephyr. The intention is that they will only be used in scripts,
// if C++ usage is desired, make a normal C++ class and then a ZephyrType wrapper class.
class ZephyrTypeBase
{
	friend class ZephyrSubsystem;
	friend class ZephyrVirtualMachine;

public:
	explicit ZephyrTypeBase( const std::string& typeName );

	virtual ~ZephyrTypeBase(){}
	virtual std::string ToString() const = 0;
	virtual void FromString( const std::string& dataStr ) = 0;
	virtual bool EvaluateAsBool() const = 0;
	virtual ZephyrTypeBase& operator=( ZephyrTypeBase const& other ) = 0;

	const std::string GetTypeName() const								{ return m_typeMetadata.GetTypeName(); }
	bool HasMemberVariable( const std::string& varName );
	bool HasMethod( const std::string& methodName );
	
	void CallMethod( const std::string& methodName, ZephyrArgs* args );

	virtual eZephyrComparatorResult Equal( ZephyrHandle other )			{ (void)other; return eZephyrComparatorResult::UNDEFINED_VAL; }

protected:
	virtual bool SetMembersFromArgs( ZephyrArgs* args ) = 0;
	virtual bool SetMember( const std::string& memberName, ZephyrHandle value ) = 0;
	virtual ZephyrHandle GetMember( const std::string& memberName ) = 0;

	// Operators
	virtual eZephyrComparatorResult Greater( ZephyrHandle other )		{ (void)other; return eZephyrComparatorResult::UNDEFINED_VAL; }
	virtual eZephyrComparatorResult GreaterEqual( ZephyrHandle other )	{ (void)other; return eZephyrComparatorResult::UNDEFINED_VAL; }
	virtual eZephyrComparatorResult Less( ZephyrHandle other )			{ (void)other; return eZephyrComparatorResult::UNDEFINED_VAL; }
	virtual eZephyrComparatorResult LessEqual( ZephyrHandle other )		{ (void)other; return eZephyrComparatorResult::UNDEFINED_VAL; }
	virtual eZephyrComparatorResult NotEqual( ZephyrHandle other );

	virtual ZephyrHandle Negate()										{  return NULL_ZEPHYR_HANDLE; }
	virtual ZephyrHandle Add( ZephyrHandle other )		{ (void)other; return NULL_ZEPHYR_HANDLE; }
	virtual ZephyrHandle Subtract( ZephyrHandle other )	{ (void)other; return NULL_ZEPHYR_HANDLE; }
	virtual ZephyrHandle Multiply (ZephyrHandle other )	{ (void)other; return NULL_ZEPHYR_HANDLE; }
	virtual ZephyrHandle Divide( ZephyrHandle other )	{ (void)other; return NULL_ZEPHYR_HANDLE; }

protected:
	ZephyrTypeMetadata m_typeMetadata;
};


//-----------------------------------------------------------------------------------------------
template <typename OBJ_TYPE>
class ZephyrType : public ZephyrTypeBase
{
public:
	explicit ZephyrType( const std::string& typeName )
		: ZephyrTypeBase( typeName )
	{}

	void BindMethod( const std::string& name, void( OBJ_TYPE::*methodPtr )( ZephyrArgs* args ) )
	{
		ZephyrTypeMethod* methodToBind = m_typeMetadata.GetMethod( name );
		GUARANTEE_OR_DIE( methodToBind != nullptr, Stringf( "Tried to bind method '%s' to an object of type '%s', but it was not found in the metadata.", name.c_str(), m_typeMetadata.GetTypeName().c_str() ) );

		methodToBind->delegate.SubscribeMethod( (OBJ_TYPE*)this, methodPtr );
	}
};


//-----------------------------------------------------------------------------------------------
class ZephyrValue
{
public:
	ZephyrValue();
	ZephyrValue( NUMBER_TYPE value );
	ZephyrValue( bool value );
	ZephyrValue( const std::string& value );
	ZephyrValue( EntityId value );
	ZephyrValue( ZephyrHandle value );

	ZephyrValue( ZephyrValue const& other );
	ZephyrValue& operator=( ZephyrValue const& other );
	ZephyrValue( ZephyrValue const&& other );
	ZephyrValue& operator=( ZephyrValue const&& other );
	~ZephyrValue();

	friend bool operator==( const ZephyrValue& lhs, const ZephyrValue& rhs );

	eValueType		GetType() const			{ return m_type; }

	float			GetAsNumber() const		{ return numberData; }
	bool			GetAsBool() const		{ return boolData; }
	std::string		GetAsString() const;
	EntityId		GetAsEntity() const		{ return entityData; }
	ZephyrHandle	GetAsUserType() const	{ return userTypeData; }
	
	bool			EvaluateAsBool();
	std::string		EvaluateAsString();
	float			EvaluateAsNumber();
	EntityId		EvaluateAsEntity();
	ZephyrHandle	EvaluateAsUserType();

	std::string		SerializeToString() const;
	void			DeserializeFromString( const std::string& serlializedStr );

private:
	void ReportConversionError( eValueType targetType );

public:
	static const ZephyrValue ERROR_VALUE;

private:
	eValueType m_type = eValueType::NONE;

	union
	{
		NUMBER_TYPE numberData;
		bool boolData;
		std::string* strData;
		EntityId entityData;
		ZephyrHandle userTypeData = NULL_ZEPHYR_HANDLE;
	};
};


//-----------------------------------------------------------------------------------------------
struct ZephyrTimer
{
public:
	Timer timer;
	EntityId targetId = INVALID_ENTITY_ID;
	std::string name;
	std::string callbackName;
	EventArgs* callbackArgs = nullptr;

public:
	ZephyrTimer( Clock* clock );
	~ZephyrTimer();
};
