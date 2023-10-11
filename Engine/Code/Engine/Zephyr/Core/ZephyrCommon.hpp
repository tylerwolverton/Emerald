#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/ObjectFactory.hpp"
#include "Engine/Core/Delegate.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Time/Timer.hpp"
#include "Engine/Zephyr/GameInterface/ZephyrSubsystem.hpp"

#include <functional>
#include <map>
#include <string>


//-----------------------------------------------------------------------------------------------
class ZephyrValue;
class ZephyrType;
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

constexpr int ERROR_ZEPHYR_ENTITY_ID = -1000;
extern std::string PARENT_ENTITY_STR;
extern std::string PARENT_ENTITY_ID_STR;
extern std::string PARENT_ENTITY_NAME_STR;
extern std::string TARGET_ENTITY_STR;
extern std::string TARGET_ENTITY_NAME_STR;

typedef std::string ZephyrTypeId;
typedef ZephyrType* ( *ZephyrTypeObjCreationFunc )( ZephyrArgs* );
typedef ObjectFactory< ZephyrType, ZephyrTypeId, ZephyrTypeObjCreationFunc, ZephyrArgs* > ZephyrTypeObjFactory;

extern ZephyrTypeObjFactory* g_zephyrTypeObjFactory;
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
	//NUMBER,
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
	friend class ZephyrType;

public:
	ZephyrTypeMetadata() = default;
	ZephyrTypeMetadata( const std::string& typeName );

	std::string GetTypeName() const									{ return m_typeName; }
	ZephyrTypeMethod* FindMethod( const std::string& methodName );

	void RegisterMember( const std::string& memberName );
	void RegisterReadOnlyMember( const std::string& memberName );
	void RegisterMethod( const std::string& methodName );

	bool DoesTypeHaveMemberVariable( const std::string& varName );
	bool DoesTypeHaveMethod( const std::string& methodName );

private:
	std::string m_typeName;
	std::vector<std::string> m_memberNames; // Make a field struct with type and name?
	std::vector<ZephyrTypeMethod> m_methods;
};


//-----------------------------------------------------------------------------------------------
// The base class for types exposed to Zephyr. The intention is that they will only be used in scripts,
// if C++ usage is desired, make a normal C++ class and then a ZephyrType wrapper class.
class ZephyrType
{
	friend class ZephyrSubsystem;

public:
	explicit ZephyrType( const std::string& typeName );

	virtual ~ZephyrType(){}
	virtual std::string ToString() const = 0;
	virtual void FromString( const std::string& dataStr ) = 0;
	virtual bool EvaluateAsBool() const = 0;

	const std::string GetTypeName() const								{ return m_typeMetadata.GetTypeName(); }
	bool DoesTypeHaveMemberVariable( const std::string& varName );
	bool DoesTypeHaveMethod( const std::string& methodName );
	
	void CallMethod( const std::string& methodName, ZephyrArgs* args );

protected:
	ZephyrTypeMetadata m_typeMetadata;
};


//-----------------------------------------------------------------------------------------------
template <typename OBJ_TYPE>
class ZephyrTypeTemplate : public ZephyrType
{
public:
	explicit ZephyrTypeTemplate( const std::string& typeName )
		: ZephyrType( typeName )
	{}

	void BindMethod( const std::string& name, void( OBJ_TYPE::*methodPtr )( ZephyrArgs* args ) )
	{
		ZephyrTypeMethod* methodToBind = m_typeMetadata.FindMethod( name );
		GUARANTEE_OR_DIE( methodToBind != nullptr, Stringf( "Tried to bind method '%s' to an object of type '%s', but it was not found in the metadata.", name.c_str(), m_typeMetadata.GetTypeName().c_str() ) );

		methodToBind->delegate.SubscribeMethod( (OBJ_TYPE*)this, methodPtr );
	}
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
	ZephyrValue( ZephyrType* value );

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
	ZephyrType*		GetAsUserType() const	{ return userTypeData; }
	
	bool			EvaluateAsBool();
	Vec2			EvaluateAsVec2();
	Vec3			EvaluateAsVec3();
	std::string		EvaluateAsString();
	float			EvaluateAsNumber();
	EntityId		EvaluateAsEntity();
	ZephyrType*		EvaluateAsUserType();

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
		Vec2 vec2Data;
		Vec3 vec3Data;
		bool boolData;
		std::string* strData = nullptr;
		EntityId entityData;
		ZephyrType* userTypeData;
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


//-----------------------------------------------------------------------------------------------
//struct ZephyrArgs
//
//{
//	ZephyrType* GetValue( const std::string& keyName );
//	void SetValue( const std::string& keyName, ZephyrType* value );
//
//private:
//	std::map< std::string, ZephyrType* >	m_keyValuePairs;
//};
