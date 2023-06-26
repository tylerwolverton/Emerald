#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Time/Timer.hpp"

#include <functional>
#include <map>
#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrValue;
class ZephyrBytecodeChunk;
class ZephyrComponent;
class ZephyrEngineEvents;
class ZephyrSubsystem;


//-----------------------------------------------------------------------------------------------
#define NUMBER_TYPE float

typedef std::map<std::string, ZephyrValue> ZephyrValueMap;
typedef std::map<std::string, ZephyrBytecodeChunk*> ZephyrBytecodeChunkMap;
typedef std::vector<ZephyrComponent*> ZephyrComponentVector;

constexpr int ERROR_ZEPHYR_ENTITY_ID = -1000;
extern std::string PARENT_ENTITY_STR;
extern std::string PARENT_ENTITY_ID_STR;
extern std::string PARENT_ENTITY_NAME_STR;
extern std::string TARGET_ENTITY_STR;
extern std::string TARGET_ENTITY_NAME_STR;

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
	NUMBER,
	// ZEPHYR TYPE TODO: Remove these/codegen
	VEC2,
	VEC3,
	BOOL,
	STRING,
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
	// ZEPHYR TYPE TODO: Remove these/codegen
	CONSTANT_VEC2,
	CONSTANT_VEC3,

	DEFINE_VARIABLE,
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

	LAST_VAL,
};

eOpCode ByteToOpCode( byte opCodeByte );
std::string ToString( eOpCode opCode );

//-----------------------------------------------------------------------------------------------
enum class eValueType
{
	NONE,
	NUMBER,
	// ZEPHYR TYPE TODO: codegen
	VEC2,
	VEC3,
	BOOL,
	STRING,
	ENTITY,
	USER_TYPE,
};

std::string ToString( eValueType valueType );


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
};


//-----------------------------------------------------------------------------------------------
struct ZephyrTypeMethod
{
//typedef void (*MethodPtr)( EventArgs* );
typedef std::function<void(EventArgs*)> MethodPtr;


public:
	std::string name;
	MethodPtr methodPtr = nullptr;

public:
	ZephyrTypeMethod( const std::string& name, MethodPtr methodPtr )
		: name( name )
		, methodPtr( methodPtr )
	{}
};


class IZephyrType;

//-----------------------------------------------------------------------------------------------
struct ZephyrTypeMetadata
{
public:
	std::string typeName;
	std::vector<std::string> memberNames; // Make a field struct with type and name?
	std::vector<ZephyrTypeMethod> methods;
	IZephyrType* prototype = nullptr;
};

#define BEGIN_REGISTER_METADATA( zephyrTypeName, className ){\
										ZephyrTypeMetadata metadata;\
										\
										className* obj = new className();\
										metadata.prototype = obj;\
										\
										obj->m_typeName = metadata.typeName = #zephyrTypeName;\
										\
										using std::placeholders::_1;\

#define REGISTER_METADATA_MEMBER( memberName ) metadata.memberNames.push_back( #memberName ); 

#define REGISTER_METADATA_METHOD( methodName, className ) metadata.methods.emplace_back( #methodName, std::bind( &className::methodName, obj, _1 ) );

#define END_REGISTER_METADATA g_zephyrSubsystem->RegisterZephyrType( metadata ); }

//-----------------------------------------------------------------------------------------------
class IZephyrType
{
	friend class ZephyrSubsystem;

public:
	virtual ~IZephyrType();
	virtual std::string					ToString() const = 0;
	virtual IZephyrType*				CloneSelf() const;
	virtual IZephyrType*				ChildCloneSelf() const = 0;

	const	std::string					GetTypeName() const				{ return m_typeName; }
	const	std::vector<std::string>	GetMemberVariableNames() const;
	//const	std::vector<std::string>	GetMethodNames() const			{ return typeMetadata.methodNames; }

	bool DoesTypeHaveMemberVariable( const std::string& varName );
	bool DoesTypeHaveMethod( const std::string& methodName );

	void CallMethod( const std::string& methodName, EventArgs* args );

protected:
	std::string m_typeName;
	std::vector<IZephyrType*> m_clones; // TODO: Make this an object pool and reuse
};


//-----------------------------------------------------------------------------------------------
// ZEPHYR TYPE TODO: Codegen this class
class ZephyrValue
{
public:
	ZephyrValue();
	ZephyrValue( NUMBER_TYPE value );
	ZephyrValue( const Vec2& value );
	ZephyrValue( const Vec3& value );
	ZephyrValue( bool value );
	ZephyrValue( const std::string& value );
	ZephyrValue( EntityId value );
	ZephyrValue( IZephyrType* value );

	ZephyrValue( ZephyrValue const& other );
	ZephyrValue& operator=( ZephyrValue const& other );
	ZephyrValue( ZephyrValue const&& other );
	ZephyrValue& operator=( ZephyrValue const&& other );
	~ZephyrValue();

	friend bool operator==( const ZephyrValue& lhs, const ZephyrValue& rhs );

	eValueType		GetType() const			{ return m_type; }

	float			GetAsNumber() const		{ return numberData; }
	Vec2			GetAsVec2() const		{ return vec2Data; }
	Vec3			GetAsVec3() const		{ return vec3Data; }
	bool			GetAsBool() const		{ return boolData; }
	std::string		GetAsString() const;
	EntityId		GetAsEntity() const		{ return entityData; }
	IZephyrType*	GetAsUserType() const	{ return userTypeData; }
	
	bool			EvaluateAsBool();
	Vec2			EvaluateAsVec2();
	Vec3			EvaluateAsVec3();
	std::string		EvaluateAsString();
	float			EvaluateAsNumber();
	EntityId		EvaluateAsEntity();
	IZephyrType*	EvaluateAsUserType();

	std::string SerializeToString() const;
	void		DeserializeFromString( const std::string& serlializedStr );

private:
	void ReportConversionError( eValueType targetType );

public:
	static const ZephyrValue ERROR_VALUE;

private:
	eValueType m_type = eValueType::NONE;

	union
	{
		NUMBER_TYPE numberData;
		Vec2 vec2Data;
		Vec3 vec3Data;
		bool boolData;
		std::string* strData = nullptr;
		EntityId entityData;
		IZephyrType* userTypeData;
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
